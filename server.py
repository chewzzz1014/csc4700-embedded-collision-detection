from fastapi import FastAPI, Request
from pydantic import BaseModel
import numpy as np
import joblib
import logging

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s - %(levelname)s - %(message)s",
    handlers=[
        logging.FileHandler("server_logs.log"),  # Logs to a file
        logging.StreamHandler()  # Logs to the console
    ]
)

# Load the trained model
try:
    model = joblib.load("trained_model/collision_clf_model.pkl")
    logging.info("Model loaded successfully.")
except Exception as e:
    logging.error(f"Failed to load the model: {e}")
    raise e

# Define the request body structure
class AccelerationData(BaseModel):
    x: list[float]
    y: list[float]
    z: list[float]

# Initialize FastAPI app
app = FastAPI()

@app.middleware("http")
async def log_request(request: Request, call_next):
    """Middleware to log incoming requests and responses."""
    body = await request.body()
    logging.info(f"Incoming request: {request.method} {request.url}\nBody: {body.decode()}")
    response = await call_next(request)
    logging.info(f"Response status: {response.status_code}")
    return response

@app.get("/predict/")
async def predict(data: AccelerationData):
    logging.info(f"Received data: x={data.x}, y={data.y}, z={data.z}")

    # Ensure all inputs have the same length
    if not (len(data.x) == len(data.y) == len(data.z)):
        error_message = "x, y, and z must have the same length"
        logging.error(error_message)
        return {"error": error_message}

    try:
        # Prepare input data as NumPy array
        raw_data = np.array([data.x, data.y, data.z]).T  # Shape (N, 3)
        logging.info(f"Raw data shape: {raw_data.shape}")

        # Feature engineering (adjust based on training steps)
        features = []
        for axis_data in raw_data.T:  # Iterate over x, y, z
            features.extend([
                np.mean(axis_data),  # Mean
                np.std(axis_data),  # Standard deviation
                np.min(axis_data),  # Minimum
                np.max(axis_data),  # Maximum
            ])
        logging.info(f"Extracted features: {features}")

        # Convert features into a NumPy array with shape (1, 12)
        input_features = np.array(features).reshape(1, -1)

        # Make a prediction
        prediction = model.predict(input_features)[0]

        # Return the prediction
        return int(prediction)

    except Exception as e:
        logging.error(f"Error during prediction: {e}")
        return {"error": "An error occurred during prediction"}

# Run the server (use `uvicorn` command to launch)
if __name__ == "__main__":
    import uvicorn
    # uvicorn.run(app, host='192.168.114.45', port=8000)
    uvicorn.run("server:app", host="192.168.114.45", port=8000)