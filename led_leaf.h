#define LED_LEAF_MAX_LEDS_PER_STRIP                    CONFIG_MAX_LEDS_PER_STRIP
#define LED_LEAF_MIN_RUNNER_START_INTERVAL_MS          200
#define LED_LEAF_DEFAULT_RUNNER_HUE_CHANGE             CONFIG_RUNNER_HUE_CHANGE
#define LED_LEAF_DEFAULT_RUNNER_HUE_CHANGE_INTERVAL_MS CONFIG_RUNNER_HUE_CHANGE_INTERVAL_MS
#define LED_LEAF_DEFAULT_RUNNER_GLOW_NUM_LEDS          CONFIG_RUNNER_GLOW_NUM_LEDS
struct LedLeaf {
    uint8_t leafID;    // ID of the leaf
    int     numLeds;    // Number of leds of the leaf

    CHSV runnerColor;    // LED color of the runner
    long runnerBaseTime;
    long runnerDiffTime;
    bool previousSenseState;
    
    long lastRunnerStartTime;

    CRGB leds[LED_LEAF_MAX_LEDS_PER_STRIP];

    RunnerCluster   *runnerCluster;
    SenseSensor     sensor;
    StoredTime      storedTime;
    Background      background;

    LedLeaf(uint8_t leafID,
            int     numLeds,
            uint8_t sendPin,
            uint8_t sensePin,
            CHSV    backgroundActiveColor,
            CHSV    backgroundInactiveColor,
            long    timePerLed,
            long    timeMinStored,
            long    timeMaxStoredOffset,
            uint8_t timeIncRatio,
            uint8_t timeDecRatio,
            CHSV    runnerColor,
            long    runnerBaseTime,
            long    runnerDiffTime,
            RunnerCluster *runnerCluster);
    void runCycle();
    void doSetup();
};
