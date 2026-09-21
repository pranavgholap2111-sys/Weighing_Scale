// =========================================================================
// CONFIGURATION (SECURE)
// =========================================================================
// 1. Fetches the bot token securely from Project Settings > Script Properties
const scriptProperties = PropertiesService.getScriptProperties();
const TELEGRAM_BOT_TOKEN = scriptProperties.getProperty('TELEGRAM_BOT_TOKEN');

// 2. Chat ID is not a secret, so it remains here (or set your specific ID)
const TELEGRAM_CHAT_ID = "YOUR_CHAT_ID_HERE"; 

function doPost(e) {
  const sheet = SpreadsheetApp.getActiveSpreadsheet().getActiveSheet();
  const data = JSON.parse(e.postData.contents);
  
  // 1. Create headers if sheet is empty
  if (sheet.getLastRow() === 0) {
    sheet.appendRow(["Trade ID", "Date", "Time", "Scale ID", "Location", "Weight (kg)"]);
  }
  
  // 2. Append row to Google Sheet
  sheet.appendRow([
    data.trade_id,
    data.date,
    data.time,
    data.scale_id,
    data.location,
    data.weight_kg
  ]);
  
  // 3. Send Telegram Notification
  sendTelegramAlert(data);
  
  return ContentService.createTextOutput(JSON.stringify({"result":"success"}))
                       .setMimeType(ContentService.MimeType.JSON);
}

function sendTelegramAlert(data) {
  const message = "<b>Poultry Trade Receipt</b>\n\n" +
                "<b>Trade ID:</b> " + data.trade_id + "\n" +
                "<b>Date:</b> " + data.date + "\n" +
                "<b>Time:</b> " + data.time + "\n" +
                "<b>Location:</b> " + data.location + "\n" +
                "<b>Scale ID:</b> " + data.scale_id + "\n" +
                "------------------------------\n" +
                "<b>Net Weight:</b> " + data.weight_kg + " kg\n" +
                "------------------------------";

  const url = "https://api.telegram.org/bot" + TELEGRAM_BOT_TOKEN + "/sendMessage";
  
  const payload = {
    "chat_id": TELEGRAM_CHAT_ID,
    "text": message,
    "parse_mode": "HTML"
  };

  const options = {
    "method": "post",
    "contentType": "application/json",
    "payload": JSON.stringify(payload)
  };

  try {
    UrlFetchApp.fetch(url, options);
  } catch (err) {
    Logger.log("Telegram Error: " + err);
  }
}
