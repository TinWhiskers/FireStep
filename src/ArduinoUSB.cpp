#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <errno.h>
#include "ArduinoUSB.h"
#include "FireLog.h"
#include "FireUtils.h"
#include "version.h"

using namespace std;
using namespace firestep;

ArduinoUSB::ArduinoUSB(const char *path) 
	: path(path) 
{}

int ArduinoUSB::open() {
    resultCode = 0;
    fd = ::open(SERIAL_PATH, O_RDONLY | O_NOCTTY | O_NONBLOCK );
    if (fd < 0) {
        cerr << "ERROR	: could not open " << SERIAL_PATH << endl;
        resultCode = fd;
    }
    if (resultCode == 0) {
        os.open(path.c_str());
        if (!os.is_open()) {
            cerr << "ERROR	: could not open " << path << " for output" << endl;
            close(fd);
            resultCode = -EIO;
        }
    }
}

ArduinoUSB::~ArduinoUSB() {
    if (isOpen()) {
        close(fd);
    }
}

int ArduinoUSB::init_stty(const char *sttyArgs) {
	struct stat fs;
	int rc = stat(path.c_str(), &fs);
	if (rc) {
		LOGERROR2("init_serial(%s) could not access serial port:%d", path.c_str(), rc);
		return rc;
	}

	char cmd[255];
	snprintf(cmd, sizeof(cmd), "stty -F %s %s", path.c_str(), sttyArgs);
	LOGINFO1("ArduinoUSB::init_stty() %s", cmd);
	rc = system(cmd);
	if (rc == 0) {
		LOGINFO1("init_serial(%s) initialized serial port", path.c_str());
	} else if (rc == 256) {
		LOGERROR2("init_serial(%s) failed. Add user to serial group and login again.", path.c_str(), rc);
	} else {
		LOGERROR2("init_serial(%s) could not initialize serial port. stty=>%d", path.c_str(), rc);
	}

	return rc;
}

bool ArduinoUSB::isOpen() {
    return resultCode == 0 ? true : false;
}

string ArduinoUSB::readln(int32_t msTimeout) {
    string line;
    bool isEOL = false;
    int32_t msIdle = millis() + msTimeout;
    char buf[10];
    do {
        int res = read(fd, buf, 1);
        if (res == 1) {
            if (buf[0] == '\r') {
                line += "\n";
                isEOL = true;
            } else if (buf[0] == '\n') {
                // do nothing
            } else {
                line += buf[0];
                msIdle = millis() + msTimeout;
            }
        } else if (res == 0) {
            //cerr << "USB	: zero bytes read" << endl;
        }
        switch (errno) {
        case 0:
            // happiness
            break;
        case -EAGAIN:
            cerr << "USB	: EAGAIN" << endl;
            break;
        default:
            cerr << "USB	: ERR" << res << endl;
			usleep(1000*1000);
            break;
        }
    } while (!isEOL && millis() < msIdle);
    return line;
}

void ArduinoUSB::writeln(string line) {
    os << line << endl;
    os.flush();
}

