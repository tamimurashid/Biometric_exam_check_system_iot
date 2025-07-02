Here's a revised `README.md` for your Biometric Exam Room Access Control System, formatted for clarity, engagement, and ease of use.

```markdown
# 🔐 Biometric Exam Room Access Control System (ESP32 + Google Sheets)

This project outlines a **crash-proof IoT fingerprint access control system** specifically designed for **exam rooms**. It leverages an **ESP32** microcontroller, an **R307 fingerprint sensor**, and a robust backend built with **Google Apps Script** and **Google Sheets** for real-time user validation and access logging.

> 💡 This system ensures secure access through fingerprint authentication, cloud-based validation via Google Sheets, and provides instant feedback on an LCD display.

---

## 🚀 Features

* **Secure Fingerprint Scanning:** Utilizes the R307 sensor for reliable fingerprint recognition.
* **WiFi Connectivity:** Enables seamless communication with Google services for cloud validation.
* **Cloud-Based Verification:** Sends user IDs and room names to Google Apps Script for real-time verification against a Google Sheet database.
* **Real-time Feedback:** An I2C 16x2 LCD display provides immediate access status and user information.
* **Audible Alerts:** A buzzer provides distinct audio feedback for granted or denied access.
* **Offline Fallback:** Basic fingerprint recognition remains functional even without an internet connection.
* **Comprehensive Logging:** All access attempts are meticulously logged in a dedicated "Logs" sheet within Google Sheets.

---

## 🧰 Hardware Components

| Component                 | Description                               |
| :------------------------ | :---------------------------------------- |
| **ESP32 Dev Board** | WiFi-enabled microcontroller              |
| **R307 Fingerprint Sensor** | For fingerprint recognition               |
| **I2C 16x2 LCD Display** | For displaying access status and messages |
| **Buzzer** | Provides audio feedback on access status  |
| **Jumper Wires + Breadboard** | For prototyping connections               |

---

## ☁️ Cloud Tools Used

| Tool                     | Purpose                                          |
| :----------------------- | :----------------------------------------------- |
| **Google Apps Script** | Handles backend logic for user verification and access logging |
| **Google Sheets** | Stores registered user data and access logs      |
| **Sheet.Best** | Facilitates fetching response data from Google Sheets after validation |

---

## 📂 Project Structure

```

biometric\_exam\_check\_system\_iot/
├── biometric\_exam\_check.ino    \# Main Arduino sketch for ESP32
├── wiring\_diagram.png          \# Optional: Visual wiring guide
├── https://www.google.com/search?q=LICENSE                     \# Open-source license for the project
├── README.md                   \# Project overview and documentation
└── docs/                       \# (Optional) Contains additional documentation or screenshots

````

---

## 🛠️ Setup Instructions

Follow these steps to set up your Biometric Exam Room Access Control System:

### 1. 📄 Google Sheets Structure

Create a new Google Sheet in your Google Drive with **two tabs** (sheets) named `Users` and `Logs`.

#### `Users` Sheet

This sheet stores registered user information.

| `user_id` | `full_name`   | `assigned_room` |
| :-------- | :------------ | :-------------- |
| `ID_1234` | John Doe      | `PG12`          |
| `ID_ABCD` | Alice Smith   | `PG13`          |
| ...       | ...           | ...             |

#### `Logs` Sheet

This sheet records all access attempts.

| `user_id` | `full_name` | `assigned_room` | `device_room` | `status`     | `date`       | `time`     |
| :-------- | :---------- | :-------------- | :------------ | :----------- | :----------- | :--------- |
| `ID_1234` | John Doe    | `PG12`          | `PG12`        | `VALID`      | `2025-07-01` | `10:23:11` |
| `ID_ABCD` | Alice       | `PG13`          | `PG12`        | `WRONG_ROOM` | `2025-07-01` | `10:30:02` |
| ...       | ...         | ...             | ...           | ...          | ...          | ...        |

---

### 2. ✍️ Google Apps Script

Open the **script editor** (Extensions > Apps Script) of your Google Sheet and paste the following JavaScript code:

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
````

#### 🚀 Deploy the Script:

1.  Click **Deploy** → **New deployment**.
2.  Select **Web app** as the type.
3.  Set "Execute as" to "Me".
4.  Set "Who has access" to "**Anyone**".
5.  Click **Deploy**.
6.  **Copy the deployment URL** (or the deployment ID) for use in your Arduino sketch.

-----

### 3\. 🔌 Sheet.Best Setup

1.  Visit [https://sheet.best](https://sheet.best).
2.  Connect your Google Sheet to Sheet.Best.
3.  **Copy the generated UUID** and integrate it into your Arduino sketch.

-----

## ⚙️ Arduino Code Highlights

In the `biometric_exam_check.ino` file, make sure to configure the following variables with your specific details:

```cpp
const char* ssid = "YOUR_SSID";             // Your WiFi network SSID
const char* password = "YOUR_PASSWORD";     // Your WiFi network password
const String deviceRoom = "PG12";           // The room this specific device is installed in (e.g., "PG12")
const String server_id = "YOUR_GAS_DEPLOYMENT_ID"; // The deployment ID from your Google Apps Script web app
const char* sheetUUID = "YOUR_SHEETBEST_UUID";   // The UUID obtained from Sheet.Best
```

The ESP32 sketch will perform the following actions:

  * Connect to your specified WiFi network.
  * Scan for a fingerprint.
  * Send the recognized fingerprint ID and the `deviceRoom` to your deployed Google Apps Script.
  * Fetch the validation results from Sheet.Best.
  * Display the access status and user name on the LCD.
  * Play an appropriate buzzer tone based on the access status.

-----

## 📟 LCD + Buzzer Feedback

The system provides clear visual and auditory feedback:

| State             | LCD Text                 | Buzzer                  |
| :---------------- | :----------------------- | :---------------------- |
| **Access Granted** | `Access Granted` + user's name | ✅ Short double beep    |
| **Wrong Room** | `Wrong Room!` + assigned room | ❌ Long low beep        |
| **User Not Found**| `Access Denied` + reason | ❌ Long low beep        |

-----

## 💡 Future Improvements

Consider enhancing the system with these features:

  * **NFC Fallback:** Implement NFC card reading as an alternative access method.
  * **Alternative Backend:** Explore using Firebase or another robust database instead of Google Sheets for scalability and performance.
  * **Real-time Dashboard:** Develop a real-time monitoring dashboard using frameworks like Flutter or React to visualize access logs and user activity.
  * **Camera Integration:** Add a camera module for motion-triggered snapshots upon access attempts, enhancing security.

-----

## 🛡️ License

This project is licensed under the [MIT License](https://www.google.com/search?q=LICENSE).

-----

## 👤 Author

**Tamimu Rashid**
Cybersecurity Student & IoT Dev
GitHub: [@tamimurashid](https://www.google.com/search?q=https://github.com/tamimurashid)

```
```