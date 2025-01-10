from fastapi import FastAPI
from pydantic import BaseModel
import numpy as np
import joblib

# Load the trained model
model = joblib.load("trained_model/collision_clf_model.pkl")

# Define the request body structure
class AccelerationData(BaseModel):
    x: list[float]
    y: list[float]
    z: list[float]

# Initialize FastAPI app
app = FastAPI()

@app.post("/predict/")
async def predict(data: AccelerationData):
    # Ensure all inputs have the same length
    if not (len(data.x) == len(data.y) == len(data.z)):
        return {"error": "x, y, and z must have the same length"}
    
    # Prepare input data as NumPy array
    raw_data = np.array([data.x, data.y, data.z]).T  # Shape (N, 3)
    
    # Feature engineering (adjust based on training steps)
    features = []
    for axis_data in raw_data.T:  # Iterate over x, y, z
        features.extend([
            np.mean(axis_data),  # Mean
            np.std(axis_data),  # Standard deviation
            np.min(axis_data),  # Minimum
            np.max(axis_data),  # Maximum
        ])
    
    # Convert features into a NumPy array with shape (1, 12)
    input_features = np.array(features).reshape(1, -1)
    
    # Make a prediction
    prediction = model.predict(input_features)[0]
    
    # Return the prediction
    return int(prediction)

# Run the server (use `uvicorn` command to launch)
if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000, reload=True)