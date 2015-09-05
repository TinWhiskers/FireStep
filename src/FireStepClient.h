#ifndef FIRESTEPCLIENT_H
#define FIRESTEPCLIENT_H

#include "ArduinoUSB.h"
#include "FireStep.h"

namespace firestep {

typedef class FireStepClient : public IFireStep {
private:
    bool prompt;
    std::string serialPath;
    int32_t msResponse; // response timeout
    firestep::ArduinoUSB usb;
protected:
	std::string readLine(std::istream &is);
    int send(std::string request, std::string &response);
public:
    FireStepClient(bool prompt=true, const char *serialPath=FIRESTEP_SERIAL_PATH, int32_t msResponse=10*1000);
	~FireStepClient();
	static std::string version(bool verbose=true);
    int reset();
    int console();
    int sendJson(std::string json);

	/* IFireStep */ protected: virtual std::string executeCore(std::string &json);
	/* IFireStep */ public: virtual int startup();
	/* IFireStep */ public: virtual int shutdown();
} FireStepClient;

}

#endif