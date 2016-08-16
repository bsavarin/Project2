'use strict';
/* eslint-disable quotes */
module.exports = [
  {
    "type": "heading",
    "id": "main-heading",
    "defaultValue": "<h3>Project #2 Configuration</h3>",
    "size": 1
  },
  {
    "type": "text",
    "defaultValue": "Use this configuration page to choose the settings you would like applied to this app."
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Controls"
      },
      {
        "type": "toggle",
        "capabilities": ["BW"],
        "messageKey": "INVERT",
        "label": "Invert watchface",
        "description": "For Pebble and Pebble Steel only.",
        "defaultValue": false
      },
      {
        "type": "toggle",
        "messageKey": "SECONDS",
        "label": "Show second hand",
        "description": " ",
        "defaultValue": true
      },
      {
        "type": "select",
        "messageKey": "TEMPERATURE_UNIT",
        "defaultValue": "0",
        "label": "Temperature Unit",
        "description": " ",
        "options": [
          { 
            "label": "Celsius",
            "value": "0"
          },
          { 
            "label": "Fahrenheit",
            "value": "1"
          }
        ],
      },
      {
        "type": "slider",
        "messageKey": "BT_VIBRATE",
        "defaultValue": 1,
        "label": "Vibration Options",
        "description": "0 = None, 1 = BT vibe, 2 = Hourly vibe, 3 = Both",
        "min": 0,
        "max": 3,
        "step": 1
      },
    ]
  },
    {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Display Options"
      },
      {
        "type": "select",
        "messageKey": "RIGHT_INFORMATION_BOX",
        "defaultValue": "0",
        "label": "Right Data",
        "description": " ",
        "options": [
          { 
            "label": "None",
            "value": "0" 
          },
          { 
            "label": "Date - choose format below",
            "value": [
               { 
                  "label": "01",
                  "value": "10" 
                },
                { 
                  "label": "SUN",
                  "value": "11" 
                },
                { 
                  "label": "SUN 01",
                  "value": "12" 
                },
                { 
                  "label": "01 JAN",
                  "value": "13" 
                },
                { 
                  "label": "JAN 01",
                  "value": "14" 
                }              
            ]
          },
        ],
      },
      {
        "type": "select",
        "capabilities": ["HEALTH"],
        "messageKey": "TOP_INFORMATION_BOX",
        "defaultValue": "0",
        "label": "Top Data",
        "description": " ",
        "options": [
          { 
            "label": "None",
            "value": "0" 
          },
          { 
            "label": "Weather Location",
            "value": "1"
          },
          { 
            "label": "Weather Conditions",
            "value": "2"
          },
          { 
            "label": "Day & Week Number",
            "value": "3"
          },
          { 
            "label": "Step Count",
            "value": "4" 
          },
          { 
            "label": "Distance",
            "value": "5" 
          },
          { 
            "label": "Sleep",
            "value": "6" 
          },
          { 
            "label": "Calories",
            "value": "7" 
          },
          { 
            "label": "Digital Clock",
            "value": "8" 
          },
          { 
            "label": "Mon 01 Jan",
            "value": "9" 
          },
          { 
            "label": "Mon Jan 01",
            "value": "10" 
          },
          { 
            "label": "High/Low Temp",
            "value": "11" 
          },
          { 
            "label": "Battery %",
            "value": "12" 
          }    
        ],
      },
      {
        "type": "select",
        "capabilities": ["NOT_HEALTH"],
        "messageKey": "TOP_INFORMATION_BOX",
        "defaultValue": "0",
        "label": "Top Data",
        "description": " ",
        "options": [
          { 
            "label": "None",
            "value": "0" 
          },
          { 
            "label": "Weather Location",
            "value": "1"
          },
          { 
            "label": "Weather Conditions",
            "value": "2"
          },
          { 
            "label": "Day & Week Number",
            "value": "3"
          },
          { 
            "label": "Digital Clock",
            "value": "8" 
          },
          { 
            "label": "Mon 01 Jan",
            "value": "9" 
          },
          { 
            "label": "Mon Jan 01",
            "value": "10" 
          },
          { 
            "label": "High/Low Temp",
            "value": "11" 
          },
          { 
            "label": "Battery %",
            "value": "12" 
          }      
        ],
      },
      {
        "type": "select",
        "capabilities": ["HEALTH"],
        "messageKey": "BOTTOM_INFORMATION_BOX",
        "defaultValue": "0",
        "label": "Bottom Data",
        "description": " ",
        "options": [
          { 
            "label": "None",
            "value": "0" 
          },
          { 
            "label": "Weather Location",
            "value": "1"
          },
          { 
            "label": "Weather Conditions",
            "value": "2"
          },
          { 
            "label": "Day & Week Number",
            "value": "3"
          },
          { 
            "label": "Step Count",
            "value": "4" 
          },
          { 
            "label": "Distance",
            "value": "5" 
          },
          { 
            "label": "Sleep",
            "value": "6" 
          },
          { 
            "label": "Calories",
            "value": "7" 
          },
          { 
            "label": "Digital Clock",
            "value": "8" 
          },
          { 
            "label": "Mon 01 Jan",
            "value": "9" 
          },
          { 
            "label": "Mon Jan 01",
            "value": "10" 
          },
          { 
            "label": "High/Low Temp",
            "value": "11" 
          },
          { 
            "label": "Battery %",
            "value": "12" 
          },
          { 
            "label": "Custom Text",
            "value": "13" 
          } 
        ],
      },
      {
        "type": "select",
        "capabilities": ["NOT_HEALTH"],
        "messageKey": "BOTTOM_INFORMATION_BOX",
        "defaultValue": "0",
        "label": "Bottom Data",
        "description": " ",
        "options": [
          { 
            "label": "None",
            "value": "0" 
          },
          { 
            "label": "Weather Location",
            "value": "1"
          },
          { 
            "label": "Weather Conditions",
            "value": "2"
          },
          { 
            "label": "Day & Week Number",
            "value": "3"
          },
          { 
            "label": "Digital Clock",
            "value": "8" 
          },
          { 
            "label": "Mon 01 Jan",
            "value": "9" 
          },
          { 
            "label": "Mon Jan 01",
            "value": "10" 
          },
          { 
            "label": "High/Low Temp",
            "value": "11" 
          },
          { 
            "label": "Battery %",
            "value": "12" 
          }, 
          { 
            "label": "Custom Text",
            "value": "13" 
          } 
        ],
      },
      {
          "type": "text",
          "id": "Location",
          "defaultValue": "<h6>'Pebble' will be displayed when there is no weather.</h6>"
      },
      {
          "type": "text",
          "id": "Conditions",
          "defaultValue": "<h6>'Pebble' will be displayed when there is no weather.</h6>"
      }, 
      {
        "type": "input",
        "messageKey": "CUSTOM_TEXT",
        "id": "CustomText",
        "label": "Enter Custom Text below:",
        "attributes": {
          "placeholder": "Max 12 characters",
          "maxlength": 12,
          "type": "text"
        }
      },
      {
        "type": "toggle",
        "messageKey": "BATTERY_BAR",
        "label": "Display Battery Bar",
        "description": " ",
        "defaultValue": false
      },
      {
        "type": "toggle",
        "messageKey": "LOW_BATTERY_MODE",
        "label": "Low Battery Mode",
        "description": "<h6>When enabled, choose the level below to activate mode.</h6>",
        "defaultValue": false
      },
      {
        "type": "slider",
        "id": "LowBatteryLevel",
        "messageKey": "LOW_BATTERY_LEVEL",
        "defaultValue": 0,
        "label": "Low battery level (0-50%)",
        "description": "<h6>Choose level to trigger the 'Low Battery' theme. Only items updated once a day or less are permitted.</h6>",
        "min": 0,
        "max": 50,
        "step": 10
      },
      {
        "type": "slider",
        "messageKey": "TICK_MARKS",
        "defaultValue": 0,
        "label": "Display Clock Digits/Ticks",
        "description": "0 = Ticks, 1 = Digits/Ticks, 2 = Less Ticks, 3 = Digits",
        "min": 0,
        "max": 3,
        "step": 1
      },
      {
        "type": "color",
        "capabilities": ["COLOR"],
        "messageKey": "BACKGROUND_COLOUR",
        "defaultValue": "000000",
        "label": "Background Colour",
        "sunlight": false
      },
      {
        "type": "color",
        "capabilities": ["COLOR"],
        "messageKey": "FOREGROUND_COLOUR",
        "defaultValue": "FFFFFF",
        "label": "Foreground Colour",
        "sunlight": false
       },
      {
        "type": "text",
        "defaultValue": "<h6>The settings above will only change on the watch after pressing the SAVE button below.</h6>",
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save"
  }
];