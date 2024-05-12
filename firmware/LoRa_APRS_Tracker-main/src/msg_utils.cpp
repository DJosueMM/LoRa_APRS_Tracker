#include <TinyGPS++.h>
#include <SPIFFS.h>
#include "APRSPacketLib.h"
#include "notification_utils.h"
#include "bluetooth_utils.h"
#include "winlink_utils.h"
#include "configuration.h"
#include "lora_utils.h"
#include "ble_utils.h"
#include "msg_utils.h"
#include "gps_utils.h"
#include "display.h"
#include "logger.h"

extern Beacon               *currentBeacon;
extern logging::Logger      logger;
extern Configuration        Config;

extern int                  menuDisplay;
extern uint32_t             menuTime;

extern bool                 messageLed;
extern uint32_t             messageLedTime;

extern bool                 digirepeaterActive;

extern int                  ackRequestNumber;

extern uint32_t             lastTxTime;

extern uint8_t              winlinkStatus;

extern bool                 wxRequestStatus;
extern uint32_t             wxRequestTime;

extern APRSPacket           lastReceivedPacket;

String  lastMessageSaved        = "";
int     numAPRSMessages         = 0;
int     numWLNKMessages         = 0;
bool    noAPRSMsgWarning        = false;
bool    noWLNKMsgWarning        = false;
String  lastHeardTracker        = "NONE";

std::vector<String>             loadedAPRSMessages;
std::vector<String>             loadedWLNKMails;
std::vector<String>             outputMessagesBuffer;
std::vector<String>             outputAckRequestBuffer;
std::vector<String>             packet25SegBuffer;
std::vector<uint32_t>           packet25SegTimeBuffer;

bool        ackRequestState     = false;
String      ackCallsignRequest  = "";
String      ackNumberRequest    = "";
uint32_t    lastMsgRxTime       = millis();
uint32_t    lastRetryTime       = millis();

bool        messageLed          = false;
uint32_t    messageLedTime      = millis();


namespace MSG_Utils {

    bool warnNoAPRSMessages() {
        return noAPRSMsgWarning;
    }

    bool warnNoWLNKMails() {
        return noWLNKMsgWarning;
    }

    String getLastHeardTracker() {
        return lastHeardTracker;
    }

    int getNumAPRSMessages() {
        return numAPRSMessages;
    }

    int getNumWLNKMails() {
        return numWLNKMessages;
    }

    void loadNumMessages() {
        if(!SPIFFS.begin(true)) {
            Serial.println("An Error has occurred while mounting SPIFFS");
            return;
        }

        File fileToReadAPRS = SPIFFS.open("/aprsMessages.txt");
        if(!fileToReadAPRS) {
            Serial.println("Failed to open APRS_Msg for reading");
            return;
        }

        std::vector<String> v1;
        while (fileToReadAPRS.available()) {
            v1.push_back(fileToReadAPRS.readStringUntil('\n'));
        }
        fileToReadAPRS.close();

        numAPRSMessages = 0;
        for (String s1 : v1) {
            numAPRSMessages++;
        }
        logger.log(logging::LoggerLevel::LOGGER_LEVEL_INFO, "Main", "Number of APRS Messages : %s", String(numAPRSMessages));
    
        File fileToReadWLNK = SPIFFS.open("/winlinkMails.txt");
        if(!fileToReadWLNK) {
            Serial.println("Failed to open Winlink_Msg for reading");
            return;
        }

        std::vector<String> v2;
        while (fileToReadWLNK.available()) {
            v2.push_back(fileToReadWLNK.readStringUntil('\n'));
        }
        fileToReadWLNK.close();

        numWLNKMessages = 0;
        for (String s2 : v2) {
            numWLNKMessages++;
        }
        logger.log(logging::LoggerLevel::LOGGER_LEVEL_INFO, "Main", "Number of Winlink Mails : %s", String(numWLNKMessages));
    }

    void loadMessagesFromMemory(String typeOfMessage) {
        File fileToRead;
        if (typeOfMessage == "APRS") {
            noAPRSMsgWarning = false;
            if (numAPRSMessages == 0) {
                noAPRSMsgWarning = true;
            } else {
                loadedAPRSMessages.clear();
                fileToRead = SPIFFS.open("/aprsMessages.txt");
            }
            if (noAPRSMsgWarning) {
                show_display("___INFO___", "", " NO APRS MSG SAVED", 1500);
            } else {
                if(!fileToRead) {
                    Serial.println("Failed to open file for reading");
                    return;
                }
                while (fileToRead.available()) {
                    loadedAPRSMessages.push_back(fileToRead.readStringUntil('\n'));
                }
                fileToRead.close();
            }
        } else if (typeOfMessage == "WLNK") {
            noWLNKMsgWarning = false;
            if (numWLNKMessages == 0) {
                noWLNKMsgWarning = true;
            } else {
                loadedWLNKMails.clear();
                fileToRead = SPIFFS.open("/winlinkMails.txt");
            }
            if (noWLNKMsgWarning) {
                show_display("___INFO___", "", " NO WLNK MAILS SAVED", 1500);
            } else {
                if(!fileToRead) {
                    Serial.println("Failed to open file for reading");
                    return;
                }
                while (fileToRead.available()) {
                    loadedWLNKMails.push_back(fileToRead.readStringUntil('\n'));
                }
                fileToRead.close();
            } 
        }    
    }

    void ledNotification() {
        uint32_t ledTimeDelta = millis() - messageLedTime;
        if (messageLed && ledTimeDelta > 5 * 1000) {
            digitalWrite(Config.notification.ledMessagePin, HIGH);
            messageLedTime = millis();
        }
        uint32_t ledOnDelta = millis() - messageLedTime;
        if (messageLed && ledOnDelta > 1 * 1000) {
            digitalWrite(Config.notification.ledMessagePin, LOW);
        }
        if (!messageLed && digitalRead(Config.notification.ledMessagePin) == HIGH) {
            digitalWrite(Config.notification.ledMessagePin, LOW);
        }
    }

    void deleteFile(String typeOfFile) {
        if(!SPIFFS.begin(true)) {
            Serial.println("An Error has occurred while mounting SPIFFS");
            return;
        }
        if (typeOfFile == "APRS") {
            SPIFFS.remove("/aprsMessages.txt");
        } else if (typeOfFile == "WLNK") {
            SPIFFS.remove("/winlinkMails.txt");
        }    
        if (Config.notification.ledMessage) {
            messageLed = false;
        }
    }

    void saveNewMessage(String typeMessage, String station, String newMessage) {
        if (typeMessage == "APRS" && lastMessageSaved != newMessage) {
            File fileToAppendAPRS = SPIFFS.open("/aprsMessages.txt", FILE_APPEND);
            if(!fileToAppendAPRS) {
                Serial.println("There was an error opening the file for appending");
                return;
            }
            newMessage.trim();
            if(!fileToAppendAPRS.println(station + "," + newMessage)) {
                Serial.println("File append failed");
            }
            lastMessageSaved = newMessage;
            numAPRSMessages++;
            fileToAppendAPRS.close();
            if (Config.notification.ledMessage) {
                messageLed = true;
            }
        } else if (typeMessage == "WLNK" && lastMessageSaved != newMessage) {
            File fileToAppendWLNK = SPIFFS.open("/winlinkMails.txt", FILE_APPEND);
            if(!fileToAppendWLNK) {
                Serial.println("There was an error opening the file for appending");
                return;
            }
            newMessage.trim();
            if(!fileToAppendWLNK.println(newMessage)) {
                Serial.println("File append failed");
            }
            lastMessageSaved = newMessage;
            numWLNKMessages++;
            fileToAppendWLNK.close();
            if (Config.notification.ledMessage) {
                messageLed = true;
            }
        }
    }

    void sendMessage(String station, String textMessage) {
        String newPacket = APRSPacketLib::generateMessagePacket(currentBeacon->callsign, "APLRT1", Config.path, station, textMessage);
        #if HAS_TFT
        cleanTFT();
        #endif
        if (textMessage.indexOf("ack") == 0 && station != "WLNK-1") {  // don't show Winlink ACK
            show_display("<<ACK Tx>>", 500);
        } else if (station.indexOf("CA2RXU-15") == 0 && textMessage.indexOf("wrl") == 0) {
            show_display("<WEATHER>","", "--- Sending Query ---",  1000);
            wxRequestTime = millis();
            wxRequestStatus = true;
        } else {
            if (station == "WLNK-1") {
                show_display("WINLINK Tx", "", newPacket, 100);
            } else {
                show_display("MSG Tx >>", "", newPacket, 100);
            }
        }
        LoRa_Utils::sendNewPacket(newPacket);
    }

    String ackRequestNumberGenerator() {
        ackRequestNumber++;
        if (ackRequestNumber > 999) {
            ackRequestNumber = 1;
        }
        return String(ackRequestNumber);
    }

    void addToOutputBuffer(uint8_t typeOfMessage, String station, String textMessage) {
        bool alreadyInBuffer;
        if (typeOfMessage == 1) {
            alreadyInBuffer = false;
            if (!outputMessagesBuffer.empty()) {
                for (int i = 0; i < outputMessagesBuffer.size(); i++) {
                    if (outputMessagesBuffer[i].indexOf(station + "," + textMessage) == 0) {
                        alreadyInBuffer = true;
                    }
                }
            }
            if (!outputAckRequestBuffer.empty()) {
                for (int j = 0; j < outputAckRequestBuffer.size(); j++) {
                    if (outputAckRequestBuffer[j].indexOf(station + "," + textMessage) > 1) {
                        alreadyInBuffer = true;
                    }
                }
            }               
            if (!alreadyInBuffer) {
                outputMessagesBuffer.push_back(station + "," + textMessage + "{" + ackRequestNumberGenerator());
            }
        } else if (typeOfMessage == 0) {
            alreadyInBuffer = false;
            if (!outputMessagesBuffer.empty()) {
                for (int k = 0; k < outputMessagesBuffer.size(); k++) {
                    if (outputMessagesBuffer[k].indexOf(station + "," + textMessage) == 0) {
                        alreadyInBuffer = true;
                    }
                }
            }
            if (!alreadyInBuffer) {
                outputMessagesBuffer.push_back(station + "," + textMessage);
            }
        }
    }

    void processOutputBuffer() {
        if (!outputMessagesBuffer.empty() && (millis() - lastMsgRxTime) >= 4500 && (millis() - lastTxTime) > 3000) {
            String addressee = outputMessagesBuffer[0].substring(0, outputMessagesBuffer[0].indexOf(","));
            String message = outputMessagesBuffer[0].substring(outputMessagesBuffer[0].indexOf(",") + 1);
            if (message.indexOf("{") > 0) {     // message with ack Request
                outputAckRequestBuffer.push_back("6," + addressee + "," + message);  // 6 is for ack packets retries
                outputMessagesBuffer.erase(outputMessagesBuffer.begin());
            } else {                            // message without ack Request
                sendMessage(addressee, message);
                outputMessagesBuffer.erase(outputMessagesBuffer.begin());
                lastTxTime = millis();
            }
        }
        if (outputAckRequestBuffer.empty()) {
            ackRequestState = false;
        } else if (!outputAckRequestBuffer.empty() && (millis() - lastMsgRxTime) >= 4500 && (millis() - lastTxTime) > 3000) {
            bool sendRetry = false;
            String triesLeft = outputAckRequestBuffer[0].substring(0 , outputAckRequestBuffer[0].indexOf(","));
            switch (triesLeft.toInt()) {
                case 6:
                    sendRetry = true;
                    ackRequestState = true;
                    break;
                case 5:
                    if (millis() - lastRetryTime > 30 * 1000) sendRetry = true;
                    break;
                case 4:
                    if (millis() - lastRetryTime > 60 * 1000) sendRetry = true;
                    break;
                case 3:
                    if (millis() - lastRetryTime > 120 * 1000) sendRetry = true;
                    break;
                case 2:
                    if (millis() - lastRetryTime > 120 * 1000) sendRetry = true;
                    break;
                case 1:
                    if (millis() - lastRetryTime > 120 * 1000) sendRetry = true;
                    break;
                case 0:
                    if (millis() - lastRetryTime > 30 * 1000) {
                        ackRequestNumber = false;
                        outputAckRequestBuffer.erase(outputAckRequestBuffer.begin());
                        if (winlinkStatus > 0 && winlinkStatus < 5) {   // if not complete Winlink Challenge Process it will reset Login process
                            winlinkStatus = 0;
                        }                     
                    }
                    break;
            }
            if (sendRetry) {
                String rest = outputAckRequestBuffer[0].substring(outputAckRequestBuffer[0].indexOf(",") + 1);
                ackCallsignRequest = rest.substring(0, rest.indexOf(","));
                String payload = rest.substring(rest.indexOf(",") + 1);
                ackNumberRequest = payload.substring(payload.indexOf("{") + 1);                
                sendMessage(ackCallsignRequest, payload);
                lastTxTime = millis();
                lastRetryTime = millis();
                outputAckRequestBuffer[0] = String(triesLeft.toInt() - 1) + "," + ackCallsignRequest + "," + payload;
            }
        }
    }

    void clean25SegBuffer() {
        if (!packet25SegTimeBuffer.empty()) {
            if (millis() - packet25SegTimeBuffer[0] > 25 * 1000) {
                packet25SegTimeBuffer.erase(packet25SegTimeBuffer.begin());
                packet25SegBuffer.erase(packet25SegBuffer.begin());
            }
        }
    }

    bool check25SegBuffer(String station, String textMessage) {
        if (!packet25SegBuffer.empty()) {
            bool shouldBeIgnored = false;
            for (int i = 0; i < packet25SegBuffer.size(); i++) {
                if (packet25SegBuffer[i].substring(0, packet25SegBuffer[i].indexOf(",")) == station && packet25SegBuffer[i].substring(packet25SegBuffer[i].indexOf(",") + 1) == textMessage) {
                    shouldBeIgnored = true;
                }
            }
            if (shouldBeIgnored) {
                return false;
            } else {
                packet25SegBuffer.push_back(station + "," + textMessage);
                packet25SegTimeBuffer.push_back(millis());
                return true;
            }
        } else {
            packet25SegBuffer.push_back(station + "," + textMessage);
            packet25SegTimeBuffer.push_back(millis());
            return true;
        }    
    }
    
    void checkReceivedMessage(ReceivedLoRaPacket packet) {
        if(packet.text.isEmpty()) {
            return;
        }
        if (packet.text.substring(0,3) == "\x3c\xff\x01") {              // its an APRS packet
            //Serial.println(packet.text); // only for debug
            lastReceivedPacket = APRSPacketLib::processReceivedPacket(packet.text.substring(3),packet.rssi, packet.snr, packet.freqError);
            if (lastReceivedPacket.sender!=currentBeacon->callsign) {

                if (check25SegBuffer(lastReceivedPacket.sender, lastReceivedPacket.message)) {

                    if (Config.bluetoothType == 0 || Config.bluetoothType == 3) { // agregar validador si cliente BLE esta conectado?
                        BLE_Utils::sendToPhone(packet.text.substring(3));
                    } else {
                        #ifdef HAS_BT_CLASSIC
                        BLUETOOTH_Utils::sendPacket(packet.text.substring(3));
                        #endif
                    }

                    if (digirepeaterActive && lastReceivedPacket.addressee!=currentBeacon->callsign) {
                        String digiRepeatedPacket = APRSPacketLib::generateDigiRepeatedPacket(lastReceivedPacket, currentBeacon->callsign);
                        if (digiRepeatedPacket == "X") {
                            logger.log(logging::LoggerLevel::LOGGER_LEVEL_WARN, "Main", "%s", "Packet won't be Repeated (Missing WIDE1-X)");
                        } else {
                            delay(500);
                            LoRa_Utils::sendNewPacket(digiRepeatedPacket);
                        }
                    }
                    lastHeardTracker = lastReceivedPacket.sender;

                    if (lastReceivedPacket.type == 1 && lastReceivedPacket.addressee == currentBeacon->callsign) {

                        if (ackRequestState && lastReceivedPacket.message.indexOf("ack") == 0) {
                            if (ackCallsignRequest == lastReceivedPacket.sender && ackNumberRequest == lastReceivedPacket.message.substring(lastReceivedPacket.message.indexOf("ack") + 3)) {
                                outputAckRequestBuffer.erase(outputAckRequestBuffer.begin());
                                ackRequestState = false;
                            }
                        }
                        if (lastReceivedPacket.message.indexOf("{") >= 0) {
                            String ackMessage = "ack" + lastReceivedPacket.message.substring(lastReceivedPacket.message.indexOf("{") + 1);
                            ackMessage.trim();
                            MSG_Utils::addToOutputBuffer(0, lastReceivedPacket.sender, ackMessage);
                            lastMsgRxTime = millis();
                            lastReceivedPacket.message = lastReceivedPacket.message.substring(0, lastReceivedPacket.message.indexOf("{"));
                        }

                        if (Config.notification.buzzerActive && Config.notification.messageRxBeep) {
                            NOTIFICATION_Utils::messageBeep();
                        }
                        if (lastReceivedPacket.message.indexOf("ping") == 0 || lastReceivedPacket.message.indexOf("Ping") == 0 || lastReceivedPacket.message.indexOf("PING") == 0) {
                            lastMsgRxTime = millis();
                            MSG_Utils::addToOutputBuffer(0, lastReceivedPacket.sender, "pong, 73!");
                        }

                        if (lastReceivedPacket.sender == "CA2RXU-15" && lastReceivedPacket.message.indexOf("WX") == 0) {    // WX = WeatherReport
                            Serial.println("Weather Report Received");
                            String wxCleaning     = lastReceivedPacket.message.substring(lastReceivedPacket.message.indexOf("WX ") + 3);
                            String place          = wxCleaning.substring(0,wxCleaning.indexOf(","));
                            String placeCleaning  = wxCleaning.substring(wxCleaning.indexOf(",")+1);
                            String summary        = placeCleaning.substring(0,placeCleaning.indexOf(","));
                            String sumCleaning    = placeCleaning.substring(placeCleaning.indexOf(",")+2);
                            String temperature    = sumCleaning.substring(0,sumCleaning.indexOf("P"));
                            String tempCleaning   = sumCleaning.substring(sumCleaning.indexOf("P")+1);
                            String pressure       = tempCleaning.substring(0,tempCleaning.indexOf("H"));
                            String presCleaning   = tempCleaning.substring(tempCleaning.indexOf("H")+1);
                            String humidity       = presCleaning.substring(0,presCleaning.indexOf("W"));
                            String humCleaning    = presCleaning.substring(presCleaning.indexOf("W")+1);
                            String windSpeed      = humCleaning.substring(0,humCleaning.indexOf(","));
                            String windCleaning   = humCleaning.substring(humCleaning.indexOf(",")+1);
                            String windDegrees    = windCleaning.substring(windCleaning.indexOf(",")+1,windCleaning.indexOf("\n"));

                            String fifthLineWR    = temperature + "C  " + pressure + "hPa  " + humidity +"%";
                            String sixthLineWR    = "(wind " + windSpeed + "m/s " + windDegrees + "deg)";
                            show_display("<WEATHER>", "From --> " + lastReceivedPacket.sender, place, summary, fifthLineWR, sixthLineWR);
                            menuDisplay = 40;
                            menuTime = millis();
                        } else if (lastReceivedPacket.sender == "WLNK-1") {
                            if (winlinkStatus == 0 && !Config.simplifiedTrackerMode) {
                                lastMsgRxTime = millis();
                                if (lastReceivedPacket.message.indexOf("ack") != 0) {
                                    saveNewMessage("APRS", lastReceivedPacket.sender, lastReceivedPacket.message);
                                }                                    
                            } else if (winlinkStatus == 1 && ackNumberRequest == lastReceivedPacket.message.substring(lastReceivedPacket.message.indexOf("ack") + 3)) {
                                logger.log(logging::LoggerLevel::LOGGER_LEVEL_INFO, "Winlink","---> Waiting Challenge");
                                lastMsgRxTime = millis();
                                winlinkStatus = 2;
                                menuDisplay = 500;
                            } else if ((winlinkStatus >= 1 || winlinkStatus <= 3) &&lastReceivedPacket.message.indexOf("Login [") == 0) {
                                WINLINK_Utils::processWinlinkChallenge(lastReceivedPacket.message.substring(lastReceivedPacket.message.indexOf("[")+1,lastReceivedPacket.message.indexOf("]")));
                                logger.log(logging::LoggerLevel::LOGGER_LEVEL_INFO, "Winlink","---> Challenge Received/Processed/Sended");
                                lastMsgRxTime = millis();
                                winlinkStatus = 3;
                                menuDisplay = 501;
                            } else if (winlinkStatus == 3 && ackNumberRequest == lastReceivedPacket.message.substring(lastReceivedPacket.message.indexOf("ack") + 3)) {
                                logger.log(logging::LoggerLevel::LOGGER_LEVEL_INFO, "Winlink","---> Challenge Ack Received");
                                lastMsgRxTime = millis();
                                winlinkStatus = 4;
                                menuDisplay = 502;
                            } else if (lastReceivedPacket.message.indexOf("Login valid for") > 0) {
                                logger.log(logging::LoggerLevel::LOGGER_LEVEL_INFO, "Winlink","---> Login Succesfull");
                                lastMsgRxTime = millis();
                                winlinkStatus = 5;
                                show_display("_WINLINK_>", "", " LOGGED !!!!", 2000);
                                menuDisplay = 5000;
                            } else if (winlinkStatus == 5 && lastReceivedPacket.message.indexOf("Log off successful") == 0 ) {
                                lastMsgRxTime = millis();
                                show_display("_WINLINK_>", "", "    LOG OUT !!!", 2000);
                                winlinkStatus = 0;
                            } else if ((winlinkStatus == 5) && (lastReceivedPacket.message.indexOf("Log off successful") == -1) && (lastReceivedPacket.message.indexOf("Login valid") == -1) && (lastReceivedPacket.message.indexOf("Login [") == -1) && (lastReceivedPacket.message.indexOf("ack") == -1)) {
                                lastMsgRxTime = millis();
                                show_display("<WLNK Rx >", "", lastReceivedPacket.message , "", 3000);
                                saveNewMessage("WLNK", lastReceivedPacket.sender, lastReceivedPacket.message);
                            } 
                        } else {
                            if (!Config.simplifiedTrackerMode) {
                                lastMsgRxTime = millis();
                                show_display("< MSG Rx >", "From --> " + lastReceivedPacket.sender, "", lastReceivedPacket.message , 3000);
                                if (lastReceivedPacket.message.indexOf("ack") != 0) {
                                    saveNewMessage("APRS", lastReceivedPacket.sender, lastReceivedPacket.message);
                                }                            
                            }
                        }
                    } else {
                        if ((lastReceivedPacket.type == 0 || lastReceivedPacket.type == 4) && !Config.simplifiedTrackerMode) {
                            GPS_Utils::calculateDistanceCourse(lastReceivedPacket.sender, lastReceivedPacket.latitude, lastReceivedPacket.longitude);
                        }
                        if (Config.notification.buzzerActive && Config.notification.stationBeep && !digirepeaterActive) {
                            NOTIFICATION_Utils::stationHeardBeep();
                        }
                    }
                }                
            }
        }   
    }

}