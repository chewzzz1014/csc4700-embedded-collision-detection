from fastapi import FastAPI
from pydantic import BaseModel
import numpy as np
import joblib

# Load the trained model
model = joblib.load("collision_clf_model.pkl")  # Replace with your model's filename

# Define the request body structure
class AccelerationData(BaseModel):
    x: float
    y: float
    z: float

# Initialize FastAPI app
app = FastAPI()

@app.post("/predict/")
async def predict(data: AccelerationData):
    # Prepare input for the model
    input_data = np.array([[data.x, data.y, data.z]])
    
    # Make a prediction
    prediction = model.predict(input_data)[0]  # Assuming binary classification (0 or 1)
    
    # Return the prediction
    return {"collision": bool(prediction)}

# Run the server (use `uvicorn` command to launch)
if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000)