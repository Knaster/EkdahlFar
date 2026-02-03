#ifndef MIDICONFIGURATIONHANDLER_HPP
#define MIDICONFIGURATIONHANDLER_HPP

#include "../src/midi.cpp"

#define CREATE_MIDI_SOURCE_CALLBACKS(name) \
    MIDIHardwareWrapper *name##source; \
    void name##OnNoteOn(byte channel, byte note, byte velocity)  { name##source->midiHandler.OnNoteOn(channel, note, velocity); } \
    void name##OnNoteOff(byte channel, byte note, byte velocity) { name##source->midiHandler.OnNoteOff(channel, note, velocity); } \
    void name##OnAfterTouchPoly(byte channel, byte note, byte pressure) { name##source->midiHandler.OnAfterTouchPoly(channel, note, pressure); } \
    void name##OnChannelAftertouch(byte channel, byte pressure)  { name##source->midiHandler.OnChannelAftertouch(channel,pressure); }; \
    void name##OnControlChange(byte channel, byte control, byte value) { name##source->midiHandler.OnControlChange(channel, control, value); } \
    void name##OnPitchBend(byte channel, int pitch)  { name##source->midiHandler.OnPitchBend(channel, pitch); } \
    void name##OnProgramChange(uint8_t channel, uint8_t program) { name##source->midiHandler.OnProgramChange(channel, program); } \

#define CONNECT_MIDI_CALLBACKS(source, name) \
    source->hardwareMIDI->setHandleNoteOn(name##OnNoteOn); \
    source->hardwareMIDI->setHandleNoteOff(name##OnNoteOff); \
    source->hardwareMIDI->setHandleAfterTouchPoly(name##OnAfterTouchPoly); \
    source->hardwareMIDI->setHandleAfterTouchChannel(name##OnChannelAftertouch); \
    source->hardwareMIDI->setHandleControlChange(name##OnControlChange); \
    source->hardwareMIDI->setHandlePitchBend(name##OnPitchBend); \
    source->hardwareMIDI->setHandleProgramChange(name##OnProgramChange);

#define CONNECT_USBMIDI_CALLBACKS(source, name) \
    source->usbMIDI->setHandleNoteOn(name##OnNoteOn); \
    source->usbMIDI->setHandleNoteOff(name##OnNoteOff); \
    source->usbMIDI->setHandleAfterTouchPoly(name##OnAfterTouchPoly); \
    source->usbMIDI->setHandleAfterTouchChannel(name##OnChannelAftertouch); \
    source->usbMIDI->setHandleControlChange(name##OnControlChange); \
    source->usbMIDI->setHandlePitchChange(name##OnPitchBend); \
    source->usbMIDI->setHandleProgramChange(name##OnProgramChange);


class MIDIHardwareWrapper {
private:
public:
    midi::MidiInterface<midi::SerialMIDI<HardwareSerial>> *hardwareMIDI = nullptr;
    usb_midi_class *usbMIDI = nullptr;

    MIDIHandler midiHandler;

    enum MIDIInterfaceType {
        hardware = 0,
        teensy_usb = 1
    };

    MIDIInterfaceType midiInterfaceType;

    MIDIHardwareWrapper(midi::MidiInterface<midi::SerialMIDI<HardwareSerial>> *inHardwareMIDI) {
        hardwareMIDI = inHardwareMIDI;
        midiInterfaceType = MIDIInterfaceType::hardware;
    }

    MIDIHardwareWrapper(usb_midi_class *inUSBMIDI) {
        usbMIDI = inUSBMIDI;
        midiInterfaceType = MIDIInterfaceType::teensy_usb;
    }
};

class MIDIConfigurationHandler : public ModuleHandler
{
public:
    SETMODULEID("midiconfigurationhandler", "mcf", "MIDI message configuration handler 1.0", eModuleType::software, false)

    MODULECOMMANDHANDLER

    CREATE_MODULE_COMMAND_FUNCTION_FWD(addConfiguration, MIDIConfigurationHandler)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(removeConfiguration, MIDIConfigurationHandler)
    CREATE_MODULE_COMMAND_FUNCTION_FWD(count, MIDIConfigurationHandler)

    CREATE_INDEX_CALLBACK_FWD(configurationIndexChanged, MIDIConfigurationHandler);

    MIDIConfigurationHandler();

    MIDIHardwareWrapper* addMIDISource(midi::MidiInterface<midi::SerialMIDI<HardwareSerial>> *inHardwareMIDI);
    MIDIHardwareWrapper* addMIDISource(usb_midi_class *inUSBMIDI);

    void dumpData(std::vector<commandResponse> *dataDump) override;

    bool addDefaultConfiguration();

    void update();
    std::vector<MIDIHardwareWrapper> midiSources;
protected:
private:

};

#endif // MIDICONFIGURATIONHANDLER_HPP
