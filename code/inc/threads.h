/*
 * This file contains all the function prototypes
 * and global structures used in threads.
 */


/*
 * Misc functions used by threads
 */

/* Last flash page is used to store settings */
#define SETTINGS_ADDRESS 0x8007C00

struct __settings {

    struct {
        uint8_t cut_time;
        uint8_t cut_type;
        uint16_t sensor_threshold;
        uint8_t sensor_gain;

    } data;
    volatile uint32_t CRCValue;
};
typedef struct __settings settings_t;



settings_t* readSettings(void);
uint8_t writeSettings(settings_t* settings);

/* End of misc functions */

/* Light */
#define LIGHT_STATE_OFF 0
#define LIGHT_STATE_STILL 1
#define LIGHT_STATE_BLINK 2
#define LIGHT_STATE_PULSE 3

void startlight(void) __attribute__ ((noreturn));
struct __light_settings {

    uint8_t state;
    uint16_t duration; /* Duration/interval in ms */
};
typedef struct __light_settings light_settings_t;
extern light_settings_t light_settings;

/* End of Light */


/* ADC */
#define ADC_STOPPED 0
#define ADC_STARTED 1

struct __adc_status {
    uint8_t status;
};
typedef struct __adc_status adc_status_t;
extern adc_status_t adc_status;
extern uint16_t adc_samples[20];

void startAdc(void);

/* End of ADC */


/* Ignition */
#define IGN_CUT_DISABLED 0
#define IGN_CUT_NORMAL 1
#define IGN_CUT_PROGRESSIVE 2

void startIgnition(void) __attribute__ ((noreturn));
struct __ign_cut {
    uint8_t type;
    uint16_t duration;
};
typedef struct __ign_cut ign_cut_t;
extern ign_cut_t ign_cut;

/* End of Ignition */


/* Display */
#define DISPLAY_OFF 0
#define DISPLAY_ON 1

void startDisplay(void) __attribute__ ((noreturn));

/* End of Display */

/* Sensors */
#define SENSORS_OFF 0
#define SENSORS_ON 1

void startSensors(void) __attribute__ ((noreturn));

/* End of Sensors */

