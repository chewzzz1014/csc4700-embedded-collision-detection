# csc4700-embedded-collision-detection
- Perform binary classification (Collision / No collision) using vehicle acceleration data (x,y,z axis acceleration) collected within 5 seconds timeframe. Data collected within same timeframe is identified by an unique `batch_id`
- Display real-time data collected and prediction made on ThinkSpeak dashboard

## Hardware & Software Used
- ADXL345 Accelerometer
- ESP-32 Wifi Module
- ThinkSpeak (Dashboard & store dataset collected)
- Google Colab (ML)

## TODO
Code
- [x] Collect accelerometer data and upload to ThinkSpeak
- [x] Preprocess dataset downloaded from ThinkSpeak (2 channel, train data channel and test data channel)
- [ ] Train and evaluate classification model
- [ ] Use trained model to make real-time prediction
- [ ] Display data and graphs on ThinkSpeak dashboard

Non-Code
- [ ] tinkercard architectural diagram
- [ ] flowchart for methodology
- [ ] other diagrams