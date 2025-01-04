from flask import Flask, request, jsonify
import joblib
import numpy as np

# Load the trained model
model = joblib.load("collision_model.pkl")

# Initialize Flask app
app = Flask(__name__)

# Prediction route
@app.route('/predict', methods=['GET'])
def predict():
    # Get data from the URL query parameters
    x = float(request.args.get('x'))
    y = float(request.args.get('y'))
    z = float(request.args.get('z'))
    
    # Prepare data for prediction
    data = np.array([[x, y, z]])
    
    # Make prediction
    prediction = model.predict(data)
    
    # Return the prediction (0 or 1)
    return jsonify({'prediction': int(prediction[0])})

if __name__ == '__main__':
    app.run(debug=True)
