function doPost(e) {
  try {
    const ss = SpreadsheetApp.getActiveSpreadsheet();
    const usersSheet = ss.getSheetByName("Users");
    const logsSheet = ss.getSheetByName("Logs");

    if (!usersSheet || !logsSheet) {
      return jsonResponse({ status: "error", message: "Sheets not found" });
    }

    // Parse raw JSON input
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
