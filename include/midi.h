#ifndef MIDI_H
#define MIDI_H

class MIDIHandler {
public:
    MIDIHandler();

    MIDIMessageConfiguration *midiMessageConfiguration = nullptr;

private:
    struct noteMsg {
        char channel = -1;
        char note = -1;
        char velocity = -1;
    };

    std::vector<noteMsg> notesHeld;

    int noteCount = 0;         ///< The current amount of keys depressed
    bool monoMode = true;

    int8_t findNote(byte channel, byte note);
    bool removeNote(byte channel, byte note);
    void addNote(byte channel, byte note, byte velocity);
    void setNote();
    void setNotes();

public:
    void OnNoteOff(byte channel, byte note, byte velocity);
    void OnNoteOn(byte channel, byte note, byte velocity);
    void OnAfterTouchPoly(byte channel, byte note, byte pressure);
    void OnChannelAftertouch(byte channel, byte pressure);
    void OnControlChange(byte channel, byte control, byte value);
    void OnPitchBend(byte channel, int pitch);
    void OnProgramChange(uint8_t channel, uint8_t program);

    void updateLocalVariables();
    void processLocalMessage(String *message);
    void midiAllNotesOff();
};

#endif // MIDI_H
