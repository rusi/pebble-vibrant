module.exports = [
  {
    "type": "heading",
    "defaultValue": "App Configuration"
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Settings"
      },
      {
        "type": "toggle",
        "messageKey": "SecondTick",
        "label": "Show Seconds",
        "defaultValue": true
      },
      {
        "type": "toggle",
        "messageKey": "TickVibrate",
        "label": "Vibrate every minute",
        "defaultValue": true
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];