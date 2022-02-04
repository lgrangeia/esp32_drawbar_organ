
#include <BLEMidi.h>
#include <ml_organ.h>

#include <config.h>

void onNoteOn(uint8_t channel, uint8_t note, uint8_t velocity, uint16_t timestamp)
{
  Serial.printf("Received note on : channel %d, note %d, velocity %d (timestamp %dms)\n", channel, note, velocity, timestamp);
  Organ_NoteOn(channel, note, velocity);
}

void onNoteOff(uint8_t channel, uint8_t note, uint8_t velocity, uint16_t timestamp)
{
  Serial.printf("Received note off : channel %d, note %d, velocity %d (timestamp %dms)\n", channel, note, velocity, timestamp);
  Organ_NoteOff(channel, note);
}

void onControlChange(uint8_t channel, uint8_t controller, uint8_t value, uint16_t timestamp)
{
    Serial.printf("Received control change : channel %d, controller %d, value %d (timestamp %dms)\n", channel, controller, value, timestamp);
}

void setup() {
    Serial.begin(115200);

    Serial.printf("Initialize Audio Interface\n");
    Audio_Setup();
    button_setup();

    Serial.println("Initializing bluetooth");
    BLEMidiClient.begin("Midi client"); // "Midi client" is the name you want to give to the ESP32

    BLEMidiClient.enableDebugging();  // Uncomment to see debugging messages from the library

    Organ_Setup(&Serial, SAMPLE_RATE);
}

void loop() {
    button_loop();
    if(!BLEMidiClient.isConnected()) {
        // If we are not already connected, we try te connect to the first BLE Midi device we find
        int nDevices = BLEMidiClient.scan();
        if(nDevices > 0) {
            if(BLEMidiClient.connect(0)) {
                Serial.println("Connection established");
                BLEMidiClient.setNoteOnCallback(onNoteOn);
                BLEMidiClient.setNoteOffCallback(onNoteOff);
                BLEMidiServer.setControlChangeCallback(onControlChange);
            } else {
                Serial.println("Connection failed");
                delay(3000);    // We wait 3s before attempting a new connection
            }
        }
    }
    else {
        int32_t mono[SAMPLE_BUFFER_SIZE];
        Organ_Process_Buf(mono, SAMPLE_BUFFER_SIZE);
        Audio_OutputMono(mono);
    }
}