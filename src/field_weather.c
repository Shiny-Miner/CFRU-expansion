#include "../include/global.h"
#include "../include/blend_palette.h"
#include "../include/field_effect.h"
#include "../include/field_weather.h"
#include "../include/task.h"
#include "../include/trig.h"
#include "../include/constants/field_weather.h"
#include "../include/constants/weather.h"
#include "../include/constants/songs.h"

struct WeatherCallbacks
{
    void (*initVars)(void);
    void (*main)(void);
    void (*initAll)(void);
    bool8 (*finish)(void);
};

extern const struct WeatherCallbacks sWeatherFuncs[];
extern void (*const gWeatherPalStateFuncs[])(void);
#define gWeatherPtr ((struct Weather*) 0x2037F34)
#define NUM_FIELD_OBJECTS 16
extern struct EventObject gEventObjects[NUM_FIELD_OBJECTS];

void Task_WeatherMain(u8 taskId)
{

    u8 i;

    if (gWeatherPtr->currWeather != gWeatherPtr->nextWeather)
    {
        if (!sWeatherFuncs[gWeatherPtr->currWeather].finish()
            && gWeatherPtr->palProcessingState != WEATHER_PAL_STATE_SCREEN_FADING_OUT)
        {
            // Finished cleaning up previous weather. Now transition to next weather.
            sWeatherFuncs[gWeatherPtr->nextWeather].initVars();
            gWeatherPtr->gammaStepFrameCounter = 0;
            gWeatherPtr->palProcessingState = WEATHER_PAL_STATE_CHANGING_WEATHER;
            gWeatherPtr->currWeather = gWeatherPtr->nextWeather;
            gWeatherPtr->weatherChangeComplete = TRUE;

            /*Set all NPCs to trigger ground in order to check for shadow
            This is done because the weather can finish changing when the player is not moving
            which may cause the shadow to not show*/
            for (i = 0; i < ARRAY_COUNT(gEventObjects); i++) {
                (&gEventObjects[i])->triggerGroundEffectsOnMove = TRUE;
            }
        }
    }
    else
    {
        sWeatherFuncs[gWeatherPtr->currWeather].main();
    }
    gWeatherPalStateFuncs[gWeatherPtr->palProcessingState]();
}

