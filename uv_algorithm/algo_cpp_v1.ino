#include <SparkFun_VEML6075_Arduino_Library.h>
VEML6075 uv; // Create a VEML6075 object

double currentMinutes;
double maxMinutes;
double maxIndex;
double med;
double lsf;
const double meds[] = {150.0, 250.0, 300.0, 450.0, 600.0, 900.0};
boolean timeUp = false;

double getLSF();
double calcMinutes(double med, double uv_index, double lsf);
double getUVIndex();
double calcSelfProtectionTime(double med, double uv_index);
double calcProtectedMinutes(double lsf, double selfProtectionTime);
int getSkinType();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Wire.begin();
  if (uv.begin() == false) {
    Serial.println("Unable to communicate with  VEML6075.");
    while (1);
  }
  Serial.println("UVA, UVB, UV Index");
  currentMinutes = 0;
  maxMinutes = 1;
  maxIndex = 0;
  med = meds[getSkinType() -1];
  Serial.println("med: " + String(med));
  lsf = getLSF();
  Serial.println("lsf: " + String(lsf));
}

void loop() {
  Serial.println("we are now in the loop");
  // put your main code here, to run repeatedly:
  delay(1000);
  if (currentMinutes > 0 && !timeUp) {
    double remainingTime = maxMinutes - currentMinutes;
    Serial.print("Verbleibende Zeit bis zum Sonnenbrand: ");
    Serial.println(remainingTime);
  }
  if (currentMinutes >= maxMinutes && !timeUp) {
    timeUp = true;
    Serial.println("Aus der Sonne!!!");
    return;
  }
  double uv_index = getUVIndex();
  if (uv_index > 2) {
    currentMinutes++;
  }
  if (int(uv_index) <= maxIndex) {
    return;
  }
  if (maxIndex == 0) {
    Serial.println("maxIndex == 0 -> Werte initial berechnet...");
    double protectedMinutes = calcMinutes(med, uv_index, lsf);
    maxMinutes = round(protectedMinutes);
    return;
  }
  double newProtectedMinutes = calcMinutes(med, uv_index, lsf);
  currentMinutes = round((currentMinutes / maxMinutes) * newProtectedMinutes);
  maxMinutes = round(newProtectedMinutes);
}

double calcMinutes(double med, double uv_index, double lsf) {
  double protectedMinutes = calcProtectedMinutes(lsf, calcSelfProtectionTime(med, uv_index));
  maxIndex = int(uv_index);
  return protectedMinutes;
}

double getUVIndex() {
/*  // hole uv index von sensor
  Serial.println("Enter UV Index:");
  double uv_index = -1;
  while (uv_index <= 0) {
    if (Serial.available()) {
      int input = Serial.parseInt();
      uv_index = double(input);
    }
  }
  return uv_index;
  */
  double uv_index = uv.index();
  Serial.println("UV-Index=" + String(uv_index));
  return uv_index;
}

double getLSF() {
  // hole lsf von user
  Serial.println("Enter LSF:");
  double lsf = -1;
  int input;
  while (lsf <= 0) {
    if (Serial.available()) {
      input = Serial.parseInt();
      lsf = double(input);
    }
  }
  if (lsf == 0.0){
    lsf = 1.0;
  }
  return lsf;
}

double calcSelfProtectionTime(double med, double uv_index) {
  return (med / (uv_index * 1.5));
}

double calcProtectedMinutes(double lsf, double selfProtectionTime) {
  return (selfProtectionTime * lsf * 0.6);
}

int getSkinType() {
  // 1 - 6
  // hole hauttyp von user
  int skinType = -1;
  Serial.println("Enter Skin Type:");
  while (skinType == -1) {
    if (Serial.available()) {
      skinType = Serial.parseInt();
    }
  }
  Serial.println("returning skintype...");
  return skinType;
}
