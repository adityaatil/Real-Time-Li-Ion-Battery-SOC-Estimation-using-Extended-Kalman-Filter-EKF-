
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_INA219.h>

#define ADC_PIN 34
#define TABLE_SIZE 201

Adafruit_INA219 ina219;

// ---------------- BATTERY ----------------

float batteryCapacity = 2600; // mAh

// ---------------- EKF PARAMETERS ----------------

float Q = 0.000001;
float R = 0.001;
float P = 0.01;
float K = 0.0;

// ---------------- SOC ----------------

float soc = 0.0;

unsigned long lastTime = 0;

// ---------------- LOOKUP TABLES ----------------

float socTable[TABLE_SIZE] = {
1.0000, 0.9950, 0.9900, 0.9850, 0.9800, 0.9750, 0.9700, 0.9650, 0.9600, 0.9550,
0.9500, 0.9450, 0.9400, 0.9350, 0.9300, 0.9250, 0.9200, 0.9150, 0.9100, 0.9050,
0.9000, 0.8950, 0.8900, 0.8850, 0.8800, 0.8750, 0.8700, 0.8650, 0.8600, 0.8550,
0.8500, 0.8450, 0.8400, 0.8350, 0.8300, 0.8250, 0.8200, 0.8150, 0.8100, 0.8050,
0.8000, 0.7950, 0.7900, 0.7850, 0.7800, 0.7750, 0.7700, 0.7650, 0.7600, 0.7550,
0.7500, 0.7450, 0.7400, 0.7350, 0.7300, 0.7250, 0.7200, 0.7150, 0.7100, 0.7050,
0.7000, 0.6950, 0.6900, 0.6850, 0.6800, 0.6750, 0.6700, 0.6650, 0.6600, 0.6550,
0.6500, 0.6450, 0.6400, 0.6350, 0.6300, 0.6250, 0.6200, 0.6150, 0.6100, 0.6050,
0.6000, 0.5950, 0.5900, 0.5850, 0.5800, 0.5750, 0.5700, 0.5650, 0.5600, 0.5550,
0.5500, 0.5450, 0.5400, 0.5350, 0.5300, 0.5250, 0.5200, 0.5150, 0.5100, 0.5050,
0.5000, 0.4950, 0.4900, 0.4850, 0.4800, 0.4750, 0.4700, 0.4650, 0.4600, 0.4550,
0.4500, 0.4450, 0.4400, 0.4350, 0.4300, 0.4250, 0.4200, 0.4150, 0.4100, 0.4050,
0.4000, 0.3950, 0.3900, 0.3850, 0.3800, 0.3750, 0.3700, 0.3650, 0.3600, 0.3550,
0.3500, 0.3450, 0.3400, 0.3350, 0.3300, 0.3250, 0.3200, 0.3150, 0.3100, 0.3050,
0.3000, 0.2950, 0.2900, 0.2850, 0.2800, 0.2750, 0.2700, 0.2650, 0.2600, 0.2550,
0.2500, 0.2450, 0.2400, 0.2350, 0.2300, 0.2250, 0.2200, 0.2150, 0.2100, 0.2050,
0.2000, 0.1950, 0.1900, 0.1850, 0.1800, 0.1750, 0.1700, 0.1650, 0.1600, 0.1550,
0.1500, 0.1450, 0.1400, 0.1350, 0.1300, 0.1250, 0.1200, 0.1150, 0.1100, 0.1050,
0.1000, 0.0950, 0.0900, 0.0850, 0.0800, 0.0750, 0.0700, 0.0650, 0.0600, 0.0550,
0.0500, 0.0450, 0.0400, 0.0350, 0.0300, 0.0250, 0.0200, 0.0150, 0.0100, 0.0050,
0.0000
};

float ocvTable[TABLE_SIZE] = {
4.1346, 4.0758, 4.0648, 4.0557, 4.0494, 4.0397, 4.0351, 4.0284, 4.0236, 4.0198,
4.0150, 4.0101, 4.0090, 4.0079, 4.0077, 4.0046, 3.9998, 3.9984, 3.9979, 3.9958,
3.9931, 3.9925, 3.9921, 3.9876, 3.9871, 3.9851, 3.9840, 3.9807, 3.9802, 3.9747,
3.9736, 3.9699, 3.9657, 3.9613, 3.9555, 3.9514, 3.9471, 3.9426, 3.9360, 3.9337,
3.9261, 3.9222, 3.9171, 3.9103, 3.9052, 3.8988, 3.8955, 3.8918, 3.8854, 3.8807,
3.8749, 3.8675, 3.8633, 3.8596, 3.8556, 3.8503, 3.8480, 3.8425, 3.8355, 3.8329,
3.8270, 3.8246, 3.8203, 3.8171, 3.8127, 3.8077, 3.8052, 3.8000, 3.7944, 3.7901,
3.7859, 3.7813, 3.7745, 3.7699, 3.7652, 3.7614, 3.7558, 3.7515, 3.7456, 3.7418,
3.7382, 3.7332, 3.7274, 3.7216, 3.7175, 3.7113, 3.7091, 3.7033, 3.7006, 3.6950,
3.6889, 3.6858, 3.6808, 3.6757, 3.6710, 3.6676, 3.6609, 3.6569, 3.6521, 3.6470,
3.6428, 3.6394, 3.6349, 3.6303, 3.6264, 3.6236, 3.6195, 3.6170, 3.6110, 3.6054,
3.6028, 3.5985, 3.5982, 3.5952, 3.5899, 3.5860, 3.5824, 3.5802, 3.5770, 3.5734,
3.5696, 3.5674, 3.5669, 3.5611, 3.5589, 3.5563, 3.5512, 3.5482, 3.5466, 3.5458,
3.5419, 3.5373, 3.5368, 3.5328, 3.5311, 3.5265, 3.5229, 3.5199, 3.5173, 3.5132,
3.5100, 3.5073, 3.5049, 3.5002, 3.4992, 3.4958, 3.4924, 3.4883, 3.4861, 3.4806,
3.4782, 3.4753, 3.4698, 3.4669, 3.4631, 3.4583, 3.4554, 3.4499, 3.4468, 3.4409,
3.4387, 3.4326, 3.4272, 3.4227, 3.4167, 3.4124, 3.4064, 3.4019, 3.3981, 3.3893,
3.3847, 3.3789, 3.3738, 3.3670, 3.3609, 3.3560, 3.3502, 3.3469, 3.3408, 3.3361,
3.3327, 3.3278, 3.3210, 3.3170, 3.3091, 3.3036, 3.2950, 3.2811, 3.2655, 3.2429,
3.2200, 3.1895, 3.1594, 3.1250, 3.0848, 3.0388, 2.9840, 2.9184, 2.8254, 2.6734,
2.4987
};



// ---------------- OCV TO SOC ----------------

float getSOC(float voltage) {

  if (voltage >= ocvTable[0]) return 100.0;

  if (voltage <= ocvTable[TABLE_SIZE - 1]) return 0.0;

  for (int i = 0; i < TABLE_SIZE - 1; i++) {

    if (voltage <= ocvTable[i] &&
        voltage >= ocvTable[i + 1]) {

      float socValue =
        socTable[i] +
        (voltage - ocvTable[i]) *
        (socTable[i + 1] - socTable[i]) /
        (ocvTable[i + 1] - ocvTable[i]);

      return socValue * 100.0;
    }
  }

  return 0.0;
}
//SOC → OCV interpolation
float getOCVfromSOC(float socPercent)
{
    float socNorm = socPercent / 100.0;

    if (socNorm >= socTable[0]) return ocvTable[0];
    if (socNorm <= socTable[TABLE_SIZE - 1]) return ocvTable[TABLE_SIZE - 1];

    for (int i = 0; i < TABLE_SIZE - 1; i++)
    {
        if (socNorm <= socTable[i] &&
            socNorm >= socTable[i + 1])
        {
            return ocvTable[i] +
                   (socNorm - socTable[i]) *
                   (ocvTable[i + 1] - ocvTable[i]) /
                   (socTable[i + 1] - socTable[i]);
        }
    }

    return ocvTable[TABLE_SIZE - 1];
}
//Calculate H = dOCV/dSOC
float getH(float socPercent)
{
    float socNorm = socPercent / 100.0;

    if (socNorm >= socTable[0])
        socNorm = socTable[0] - 0.0001;

    if (socNorm <= socTable[TABLE_SIZE - 1])
        socNorm = socTable[TABLE_SIZE - 1] + 0.0001;

    for (int i = 0; i < TABLE_SIZE - 1; i++)
    {
        if (socNorm <= socTable[i] &&
            socNorm >= socTable[i + 1])
        {
            return (ocvTable[i + 1] - ocvTable[i]) /
                   ((socTable[i + 1] - socTable[i]) * 100.0);
        }
    }

    return 0.01;
}
// ---------------- SETUP ----------------

void setup() {

  Serial.begin(9600);

  analogSetPinAttenuation(ADC_PIN, ADC_11db);

  ina219.begin();

  // Initial voltage

  int adcValue = analogRead(ADC_PIN);

  float v_adc = (adcValue / 4095.0) * 3.3;

  float batteryVoltage = v_adc * 2.0;

  batteryVoltage *= 1.10;

  // Initial SOC from OCV

  soc = getSOC(batteryVoltage);

  Serial.print("Initial SOC = ");

  Serial.println(soc);

  lastTime = millis();
}

// ---------------- LOOP ----------------

void loop() {

  // -------- VOLTAGE --------

  
  int N=64;
 float sum = 0;
for(int i=0;i<N;i++)
  { 
    int adcValue = analogRead(ADC_PIN);

    float v_adc = (adcValue / 4095.0) * 3.3;

    float batteryVoltage_ini = v_adc * 2.0;

    batteryVoltage_ini *= 1.10;
    
    sum += batteryVoltage_ini;
  }
float batteryVoltage = sum/N;

  // -------- CURRENT --------

  float current_mA = ina219.getCurrent_mA();

  // -------- TIME --------

  unsigned long now = millis();

  float dt = (now - lastTime) / 3600000.0;

  lastTime = now;
  // ==================================================
  //               EKF PREDICTION
  // ==================================================

  // Coulomb counting prediction

  float soc_pred =
    soc - (current_mA * dt / batteryCapacity) * 100.0;

  // Update covariance

  P = P + Q;

  // ==================================================
  //              EKF MEASUREMENT
  // ==================================================

 float current_A =
current_mA / 1000.0;

float V_est =
getOCVfromSOC(soc_pred)
-
current_A * 0.015;

float H = getH(soc_pred);

K =
    (P * H) /
    (H * P * H + R);

soc =
    soc_pred +
    K * (batteryVoltage-V_est);

P =
    (1 - K * H) * P;

  // Limit SOC

  if (soc > 100) soc = 100;

  if (soc < 0) soc = 0;

  // ==================================================
  //                  PRINT
  // ==================================================

  Serial.print("Voltage: ");

  Serial.print(batteryVoltage, 3);

  Serial.print(" | V_est: ");
  
  Serial.print(V_est, 3);

  Serial.print(" | innovation: ");
  
  Serial.print(batteryVoltage - V_est, 3);

  Serial.print(" V | Current: ");

  Serial.print(current_mA);

  Serial.print(" mA | SOC: ");

  Serial.print(soc, 2);

  Serial.print(" % | K: ");

  Serial.print(K, 4);
  
  Serial.print(" | H: ");
  
  Serial.print(H, 3);

  Serial.print(" | P: ");

  Serial.println(P, 6);

  delay(1000);
}




