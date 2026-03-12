#ifndef PLUGIN_MULT_CPP
#define PLUGIN_MULT_CPP

#include "plugins/plugin_mult.hpp"

const ModuleCommandDeclaration Plugin_Mult::moduleCommands[] = {
    { "name", "na", "name", "Sets the map name", false, true, &s_name, eCommandType_data::ectSimpleString | eCommandType_function::ectName },
    { "target", "tg", "commandlist*", "Sets the command string to execute whenever a connection value changes", false, true, &s_target,
        eCommandType_data::ectOutputAssignment | eCommandType_dataOptions::ectExpression | eCommandType_function::ectAssignment, "mltout" },
    { "data", "da", "(adder|ratio:id:value)", "Adds or requests data", false, true, &s_data, eCommandType_data::ectData | eCommandType_function::ectParameter },
    { "remove", "rm", "adder|ratio:id", "Removes the adder or ratio with the given id", false, true, &s_remove, eCommandType_data::ectSimpleUInt8 | eCommandType_function::ectRemove }
};

getModuleCount(Plugin_Mult)

Plugin_Mult::Plugin_Mult() { }

CREATE_GETSET_FUNCTION(name, Plugin_Mult, pName)

CREATE_MODULE_COMMAND_FUNCTION(remove, Plugin_Mult) {
    String out = thisItem.shortCommand + ":";
    int i;

    if (!request) {
        i = inCommandItem->argument.size();
        if ((i  % 2) != 0) { return eProcessResult::WrongArgumentCount; }
        int j = 0; //, k = 0;
        std::vector<Plugin_Mult_Connector> *pmc;
        while ((j + 1) < i) {
            if (inCommandItem->argument[j] == "adder") {
                pmc = &addConnectors;
            } else
            if (inCommandItem->argument[j] == "ratio") {
                pmc = &ratioConnectors;
            }

            int index = -1;
            for (int k = 0; k < pmc->size(); k++) {
                if ((*pmc)[k].id == inCommandItem->argument[j + 1]) { index = i; break; }
            }
            if (index == -1) { return eProcessResult::WrongArgumentValue; }

//            debugPrintln("Removing index " + String(index), debugPrintType::Debug);
            pmc->erase(pmc->begin() + index);
            j += 2;
        }
        updateManual();
    } else {
        inCommandResponses->push_back({ thisItem.shortCommand + ":0", debugPrintType::InfoRequest });
        return eProcessResult::Ok;
    }
    inCommandResponses->push_back({ thisItem.shortCommand + ":1", debugPrintType::InfoRequest });
    return eProcessResult::Ok;
}

CREATE_MODULE_COMMAND_FUNCTION(data, Plugin_Mult) {
    String out = thisItem.shortCommand + ":";
    int i;

    if (!request) {
        i = inCommandItem->argument.size();
        if ((i  % 3) != 0) { return eProcessResult::WrongArgumentCount; }
        int j = 0; //, k = 0;
        std::vector<Plugin_Mult_Connector> *pmc;
        while ((j + 2) < i) {
            if (inCommandItem->argument[j] == "adder") {
                pmc = &addConnectors;
            } else
            if (inCommandItem->argument[j] == "ratio") {
                pmc = &ratioConnectors;
            } else {
                return eProcessResult::WrongArgumentValue;
            }
            connect(inCommandItem->argument[j + 1], inCommandItem->argument[j + 2].toFloat(), false, pmc);
            j += 3;
        }
        updateManual();
    }

    for (i = 0; i < addConnectors.size(); i++ ) {
        inCommandResponses->push_back({ out + sMultiTypeName[eMultiType::pAdder] + ":" + addConnectors[i].id + ":" + addConnectors[i].value, debugPrintType::InfoRequest });
    }
    for (i = 0; i < ratioConnectors.size(); i++ ) {
        inCommandResponses->push_back({ out + sMultiTypeName[eMultiType::pRatio] + ":" + ratioConnectors[i].id + ":" + ratioConnectors[i].value, debugPrintType::InfoRequest });
    }
    if ((addConnectors.size() == 0) && (ratioConnectors.size() == 0)) {
        inCommandResponses->push_back({ out + "0" });
    }

    return eProcessResult::Ok;
}

CREATE_GETSET_FUNCTION_F(target, Plugin_Mult, pTarget, update)

void Plugin_Mult::updateManual() {
    double out = 0, ratio = 1;
    int i;
    for (i = 0; i < addConnectors.size(); i++) { out += addConnectors[i].value; }
//    debugPrintln(String(out), debugPrintType::Debug);
    for (i = 0; i < ratioConnectors.size(); i++) { ratio *= ((double) ratioConnectors[i].value) / 65535; }
    if (ratioConnectors.size() == 0) { ratio = 1; }
//    debugPrintln(String(ratio), debugPrintType::Debug);
    out *= ratio;
    if (out < 0) { out = 0; }
//    if ((newTarget != lastOutput)) {
    if (pLastOutput != ((int32_t) out)) {
        String newTarget = pTarget;
        newTarget = newTarget.replace("mltout", String(out));
        debugPrintln(newTarget, debugPrintType::Debug);
        globalResponseCommands.addCommands(newTarget);
        //lastOutput = newTarget;
        pLastOutput = ((int32_t) out);
    }
}

Plugin_Mult_Connector* Plugin_Mult::connect(String id, float value, bool request, std::vector<Plugin_Mult_Connector> *connectors) {
    Plugin_Mult_Connector *pmc = getConnector(id, connectors);
    if (pmc == nullptr) {
        if (request) { return nullptr; }
        connectors->push_back({ id, value });
        pmc = &((*connectors)[connectors->size() - 1]);
    } else
    if (!request) {
        pmc->value = value;
    }
    return pmc;
};

Plugin_Mult_Connector* Plugin_Mult::getConnector(String inId, std::vector<Plugin_Mult_Connector> *connectors) {
    for (int i = 0; i < connectors->size(); i++) {
        if ((*connectors)[i].id == inId) {
            return &((*connectors)[i]);
        }
    }
    return nullptr;
};

#endif
