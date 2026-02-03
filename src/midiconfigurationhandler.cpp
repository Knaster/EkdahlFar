#ifndef MIDICONFIGURATIONHANDLER_CPP
#define MIDICONFIGURATIONHANDLER_CPP

#include "midiconfigurationhandler.hpp"

const ModuleCommandDeclaration MIDIConfigurationHandler::moduleCommands[] = {
    { "allnotesoff", "ano", "1|0", "Clear the entire buffer of MIDI notes held", false, false, nullptr, eCommandType::Conditional },
    { "add", "a", "(name):(event:commands)*", "Adds a new MIDI configuration with the given name, events and command strings", false, false, &s_addConfiguration, eCommandType::Data },
    { "remove", "rm", "int", "Remove the specified MIDI configuration", false,  false, &s_removeConfiguration, eCommandType::Remove },
    { "count", "c", "-", "Returns the number of MIDI configurations", false, false, &s_count, eCommandType::Count }
};

getModuleCount(MIDIConfigurationHandler)

MIDIConfigurationHandler::MIDIConfigurationHandler()
{
//    moduleID = new ModuleID("midiconfigurationhandler", "mcf", "MIDI message configuration handler 1.0", eModuleType::software);
    MIDIMessageConfiguration tempSeries;
//    ModuleGroup *group = addGroup(tempSeries.moduleID);
    ModuleGroup *group = addGroup(tempSeries.tmoduleID);
    group->mustHaveSelection = true;
    group->singleSelection = true;
    group->setIndexCallback(this, &s_configurationIndexChanged);
    addDefaultConfiguration();
    group->setSelection(0);
}

CREATE_INDEX_CALLBACK(configurationIndexChanged, MIDIConfigurationHandler) {
    for (int i = 0; i < midiSources.size(); i++) {
        midiSources[i].midiHandler.midiMessageConfiguration = moduleGroup->modules[moduleGroup->selection[0]];
    }
    debugPrintln("Current configuration set to " + moduleGroup->getSelectionLiteral(), debugPrintType::Debug);
    return true;
}

CREATE_MODULE_COMMAND_FUNCTION(addConfiguration, MIDIConfigurationHandler) {
    if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }
    //midiConfigurations.push_back(MIDIMessageConfiguration());
    //uint16_t index = midiConfigurations.size() - 1;
    //*midiConfigurations[index].name = inCommandItem->argument[0];
    MIDIMessageConfiguration *midiConfig = new MIDIMessageConfiguration();
    *midiConfig->name = inCommandItem->argument[0];
    addModule(midiConfig);
    inCommandResponses->push_back({ thisItem.shortCommand + ":1", InfoRequest });
    return eProcessResult::Ok;
}

//eProcessResult MIDIConfigurationHandler::removeConfiguration(ModuleCommandDeclarationArguments)
CREATE_MODULE_COMMAND_FUNCTION(removeConfiguration, MIDIConfigurationHandler) {
    if (!checkArguments(inCommandItem, inCommandResponses, 1)) { return eProcessResult::WrongArgumentCount; }

    ModuleGroup *group = getGroup("midiconfiguration");
    if (group == nullptr) { return eProcessResult::CommandFailed; }

    if (!request) {
        if (!group->removeModule(inCommandItem->argument[0].toInt())) {
            return eProcessResult::CommandFailed;
        }
    }
    inCommandResponses->push_back({ thisItem.shortCommand + ":1", debugPrintType::InfoRequest});
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(count, MIDIConfigurationHandler) {
    ModuleGroup *group = getGroup("midiconfiguration");
    if (group == nullptr) { return eProcessResult::CommandFailed; }

    inCommandResponses->push_back({ thisItem.shortCommand + ":" + String(group->modules.size()), debugPrintType::InfoRequest});
    return eProcessResult::Ok;
}

MIDIHardwareWrapper* MIDIConfigurationHandler::addMIDISource(midi::MidiInterface<midi::SerialMIDI<HardwareSerial>> *inHardwareMIDI) {
    MIDIHardwareWrapper source(inHardwareMIDI);
    midiSources.push_back(source);
    return &midiSources.back();
}

MIDIHardwareWrapper* MIDIConfigurationHandler::addMIDISource(usb_midi_class *inUSBMIDI) {
    MIDIHardwareWrapper source(inUSBMIDI);
    midiSources.push_back(source);
    return &midiSources.back();
}

void MIDIConfigurationHandler::update() {
}

bool MIDIConfigurationHandler::addDefaultConfiguration() {
    addModule(new MIDIMessageConfiguration());
    return true;
}

void MIDIConfigurationHandler::dumpData(std::vector<commandResponse> *inCommandResponses) {
    ModuleHandler::dumpData(inCommandResponses);
    inCommandResponses->push_back({ "mc[" + String(moduleGroups[0].selection[0]) + "]", debugPrintType::InfoRequest });
}

#endif
