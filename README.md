# 🔐 Biometric Exam Room Access Control System (ESP32 + Google Sheets)

This is a **crash-proof IoT fingerprint access control system** designed for **exam rooms**, using an **ESP32**, **R307 fingerprint sensor**, and **Google Apps Script + Sheet.Best** for user validation and access logging in real-time.

> 💡 This project enables secure access using fingerprints, cloud validation from Google Sheets, and real-time feedback on an LCD display.

---

## 🚀 Features

- 🔒 Secure fingerprint scanning with R307
- 🌐 WiFi-enabled communication with Google services
- 📤 Sends user ID + room name to Google Apps Script
- ✅ Real-time verification against a Google Sheet
- 🧠 Uses Sheet.Best to fetch response data
- 📟 LCD display shows access status and user name
- 🔊 Buzzer for audio feedback (granted or denied)
- 🧱 Offline fallback (basic fingerprint recognition still works)
- 📝 Logs all access attempts in a "Logs" sheet

---

## 🧰 Hardware Components

| Component              | Description                             |
|------------------------|-----------------------------------------|
| ESP32 Dev Board        | WiFi-enabled microcontroller            |
| R307 Fingerprint Sensor| For fingerprint recognition             |
| I2C 16x2 LCD Display   | For status display                      |
| Buzzer                 | To give feedback on access status       |
| Jumper Wires + Breadboard | For prototyping connections         |

---

## 📡 Cloud Tools Used

| Tool              | Purpose                                 |
|-------------------|------------------------------------------|
| **Google Apps Script** | Verifies fingerprint UID and logs access |
| **Google Sheets**      | Stores registered users + logs        |
| **Sheet.Best**         | Fetches response data after validation |

---

## 📂 Project Structure



biometric_exam_check_system_iot/
├── biometric_exam_check.ino # Main Arduino sketch
├── wiring_diagram.png # Optional: wiring visual
├── LICENSE # Open source license
├── README.md # Project overview
└── docs/ # (Optional) Extra documentation/screenshots



---

## 🛠️ Setup Instructions

### 1. 📄 Google Sheets Structure

Create a Google Sheet with **two tabs**:

#### Sheet: `Users`

| user_id | full_name     | assigned_room |
|---------|---------------|----------------|
| ID_1234 | John Doe      | PG12           |
| ID_ABCD | Alice Smith   | PG13           |

#### Sheet: `Logs`

| user_id | full_name | assigned_room | device_room | status      | date       | time     |
|---------|-----------|----------------|-------------|-------------|------------|----------|
| ID_1234 | John Doe  | PG12           | PG12        | VALID       | 2025-07-01 | 10:23:11 |
| ID_ABCD | Alice     | PG13           | PG12        | WRONG_ROOM  | 2025-07-01 | 10:30:02 |

---

### 2. ✍️ Google Apps Script

Paste the following in the **script editor** of the spreadsheet:

```javascript
function doPost(e) {
  try {
    const ss = SpreadsheetApp.getActiveSpreadsheet();
    const usersSheet = ss.getSheetByName("Users");
    const logsSheet = ss.getSheetByName("Logs");

    if (!usersSheet || !logsSheet) {
      return jsonResponse({ status: "error", message: "Sheets not found" });
    }

    const data = JSON.parse(e.postData.contents);
    const userID = data.user_id;
    const roomName = data.room_name;

    if (!userID || !roomName) {
      return jsonResponse({
        status: "error",
        message: "Missing 'user_id' or 'room_name' in JSON"
      });
    }

    const usersData = usersSheet.getDataRange().getValues();
    let matchedUser = null;

    for (let i = 1; i < usersData.length; i++) {
      if (usersData[i][0] === userID) {
        matchedUser = usersData[i];
        break;
      }
    }

    const now = new Date();
    const date = Utilities.formatDate(now, "GMT+3", "yyyy-MM-dd");
    const time = Utilities.formatDate(now, "GMT+3", "HH:mm:ss");

    if (matchedUser) {
      const fullName = matchedUser[1] || "";
      const assignedRoom = (matchedUser[2] || "").trim();
      const roomMatch = assignedRoom.toLowerCase() === roomName.toLowerCase();
      const status = roomMatch ? "VALID" : "WRONG_ROOM";

      logsSheet.appendRow([
        userID,
        fullName,
        assignedRoom,
        roomName,
        status,
        date,
        time
      ]);

      return jsonResponse({
        status: "success",
        user_id: userID,
        name: fullName,
        assigned_room: assignedRoom,
        device_room: roomName,
        valid: roomMatch,
        message: roomMatch
          ? "Access granted: User assigned to this room"
          : "Access denied: User assigned to a different room"
      });

    } else {
      usersSheet.appendRow([userID, "", "", ""]);
      logsSheet.appendRow([userID, "", "", roomName, "PENDING", date, time]);

      return jsonResponse({
        status: "pending",
        user_id: userID,
        device_room: roomName,
        valid: false,
        message: "User not registered. Added to Users sheet for review."
      });
    }

  } catch (err) {
    return jsonResponse({ status: "error", message: err.message });
  }
}

function jsonResponse(obj) {
  return ContentService
    .createTextOutput(JSON.stringify(obj))
    .setMimeType(ContentService.MimeType.JSON);
}

```
###🚀 Deploy it:

   # Click Deploy → Test deployments → Web app

   # Set access to “Anyone”

   # Copy the deployment URL or the deployment ID

3. 🔌 Sheet.Best Setup

    Visit https://sheet.best

    Connect your Google Sheet

    Copy the UUID and plug it into your .ino sketch



⚙️ Arduino Code Highlights

In biometric_exam_check.ino, configure:

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";
const String deviceRoom = "PG12";
const String server_id = "YOUR_GAS_DEPLOYMENT_ID";
const char* sheetUUID = "YOUR_SHEETBEST_UUID";




The ESP32:

    Connects to WiFi

    Scans fingerprint

    Sends ID and room name to Google Script

    Fetches results from Sheet.Best

    Shows result on LCD

    Plays buzzer tone

📟 LCD + Buzzer Feedback
State	LCD Text	Buzzer
Access Granted	Access Granted + name	✅ Short double beep
Wrong Room	Wrong Room! + assigned	❌ Long low beep
User Not Found	Access Denied + reason	❌ Long low beep
💡 Future Improvements

    Add NFC fallback

    Use Firebase instead of Sheets

    Add real-time dashboard with Flutter or React

    Integrate camera snapshot with motion detection

🛡 License

MIT License - see LICENSE
👤 Author

Tamimu Rashid
Cybersecurity Student & IoT Dev
GitHub: @tamimurashid
