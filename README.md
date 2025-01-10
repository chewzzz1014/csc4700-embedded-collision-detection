# csc4700-embedded-collision-detection
- Perform binary classification (Collision / No collision) using vehicle acceleration data (x,y,z axis acceleration) collected using accelerometer sensor within 5 seconds timeframe. Data collected within same timeframe is identified by an unique `batch_id`
- Display real-time data collected and prediction made on ThinkSpeak dashboard

# How to launch Trained-Model-Hosting Server
1. Activate virtual env (if any)
```
source venv/Scripts/activate
```
2. Install required packages (for first time user)
```
pip install -r requirements.txt
```
3. Run server
```
uvicorn server:app --host 0.0.0.0 --port 8000
```

## Hardware & Software Used
- ADXL345 Accelerometer
- ESP-32 Wifi Module
- ThinkSpeak (Dashboard & store dataset collected)
- Google Colab (ML)

## TODO
Code
- [x] Collect accelerometer data and upload to ThinkSpeak
- [x] Preprocess dataset downloaded from ThinkSpeak (2 channel, train data channel and test data channel)
- [x] Train and evaluate classification model
- [x] Host trained model using FastAPI
- [ ] Use trained model to make real-time prediction
- [ ] Display data and graphs on ThinkSpeak dashboard

Non-Code
- [ ] tinkercard architectural diagram
- [ ] flowchart for methodology
- [ ] other diagrams