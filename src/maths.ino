
int32_t calculateMagnitude(int32_t axisX, int32_t axisY) {

  const uint8_t deadzone = 40;

  // Apply deadzone to eliminate small movements
  if (abs(int(axisX)) < deadzone) {
    axisX = 0;
  }
  if (abs(int(axisY)) < deadzone) {
    axisY = 0;
  }

  int32_t mappedX = map(axisX, -512, 512, -100, 100);
  int32_t mappedY = map(axisY, -512, 512, -100, 100);

  mappedX = abs(mappedX);
  mappedY = abs(mappedY);

  int32_t magnitude = max(mappedX, mappedY);

  //Serial.print("calculateMagnitude:");
  //Serial.print(magnitude);
  //Serial.println("");

  return magnitude;
}

uint8_t calculateAxisHour(int32_t axisX, int32_t axisY) {

  //  Serial.print("calculateAxisHour:");
  //  Serial.print(axisRX);
  //  Serial.print(",");
  //  Serial.println(axisRY);

  const uint8_t deadzone = 40;

  // Apply deadzone to eliminate small movements
  if (abs(int(axisX)) < deadzone) {
    axisX = 0;
  }
  if (abs(int(axisY)) < deadzone) {
    axisY = 0;
  }

  if (axisX == 0 && axisY == 0) {
    return 0;  //in the deadzone on both axis
  } else {

    double atan2Value = atan2(axisX, axisY);

    int angle = floor(atan2Value * 180 / PI);

    if (angle < 0) {
      angle += 360;
    }

    //Serial.print("Angle:");
    //Serial.println(angle);

    // Convert rotated angle to hour format (1-12)
    int hourAngle = floor((angle / 30) % 12);  // Ensures 1-12 format

    if (hourAngle == 0) {
      hourAngle = 12;
    }

    //    Serial.print("Hour:");
    //  Serial.println(hourAngle);

    //  blah.innerHTML += "Hour:" + hourAngle + "<br>";
    return hourAngle;
  }
}
