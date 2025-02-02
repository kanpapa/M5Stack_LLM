/*
 *  M5_KWS_ASR_IR_TTS
 *  Voice-recognition infrared remote controller
 *  @kanpapa 2025-02-02
 *  License: MIT
 */   
/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <M5Unified.h>
#include <M5ModuleLLM.h>

// M5Core2
//const uint16_t kIrLed = 26; // M5Core2 IR Send GPIO 26
// M5CoreS3
const uint16_t kIrLed = 9; // M5CoreS3 IR Send GPIO 9

IRsend irsend(kIrLed);  // Set the GPIO to be used to sending the message.

M5ModuleLLM module_llm;
String tts_work_id;

/* Must be capitalized */
String wake_up_keyword = "HELLO";
// String wake_up_keyword = "你好你好";
String kws_work_id;
String asr_work_id;
String language;

void setup()
{
    irsend.begin();
    M5.begin();
    M5.Display.setTextSize(2);
    M5.Display.setTextScroll(true);
    // M5.Display.setFont(&fonts::efontCN_12);  // Support Chinese display

    language = "en_US";
    // language = "zh_CN";

    /* Init module serial port */
    // Serial2.begin(115200, SERIAL_8N1, 16, 17);  // Basic
    // Serial2.begin(115200, SERIAL_8N1, 13, 14);  // Core2
    Serial2.begin(115200, SERIAL_8N1, 18, 17);  // CoreS3

    /* Init module */
    module_llm.begin(&Serial2);

    /* Make sure module is connected */
    M5.Display.printf(">> Check ModuleLLM connection..\n");
    while (1) {
        if (module_llm.checkConnection()) {
            break;
        }
    }

    /* Reset ModuleLLM */
    M5.Display.printf(">> Reset ModuleLLM..\n");
    module_llm.sys.reset();

    /* Setup Audio module */
    M5.Display.printf(">> Setup audio..\n");
    module_llm.audio.setup();

    /* Setup KWS module and save returned work id */
    M5.Display.printf(">> Setup kws..\n");
    m5_module_llm::ApiKwsSetupConfig_t kws_config;
    kws_config.kws = wake_up_keyword;
    kws_work_id    = module_llm.kws.setup(kws_config, "kws_setup", language);

    /* Setup ASR module and save returned work id */
    M5.Display.printf(">> Setup asr..\n");
    m5_module_llm::ApiAsrSetupConfig_t asr_config;
    asr_config.input = {"sys.pcm", kws_work_id};
    asr_work_id      = module_llm.asr.setup(asr_config, "asr_setup", language);

    /* Setup TTS module and save returned work id */
    M5.Display.printf(">> Setup tts..\n\n");
    m5_module_llm::ApiTtsSetupConfig_t tts_config;
    tts_work_id = module_llm.tts.setup(tts_config, "tts_setup", language);

    M5.Display.printf(">> Setup ok\n>> Say \"%s\" to wakeup\n", wake_up_keyword.c_str());
}

void loop()
{
    /* Update ModuleLLM */
    module_llm.update();

    /* Handle module response messages */
    for (auto& msg : module_llm.msg.responseMsgList) {
        /* If KWS module message */
        if (msg.work_id == kws_work_id) {
            M5.Display.setTextColor(TFT_GREENYELLOW);
            M5.Display.printf(">> Keyword detected\n");
        }

        /* If ASR module message */
        if (msg.work_id == asr_work_id) {
            /* Check message object type */
            if (msg.object == "asr.utf-8.stream") {
                /* Parse message json and get ASR result */
                JsonDocument doc;
                deserializeJson(doc, msg.raw_msg);
                String asr_result = doc["data"]["delta"].as<String>();

                M5.Display.setTextColor(TFT_YELLOW);
                M5.Display.printf(">> %s\n", asr_result.c_str());

                if (asr_result == " good morning"){
                  M5.Display.printf(">> OK. LIGHT ON\n");
                  module_llm.tts.inference(tts_work_id, "OK. LIGHT ON.",8000);
                  irsend.sendPanasonic64(0x344A9034A4, 40);  // LIGHT ON
                }
                if (asr_result == " good night"){
                  M5.Display.printf(">> OK. LIGHT OFF\n");
                  module_llm.tts.inference(tts_work_id, "OK. LIGHT OFF.",8000);
                  irsend.sendPanasonic64(0x344A90F464, 40);  // LIGHT OFF
                }
            }
        }
    }

    /* Clear handled messages */
    module_llm.msg.responseMsgList.clear();
}