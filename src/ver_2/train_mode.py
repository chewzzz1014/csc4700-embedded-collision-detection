import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.linear_model import LogisticRegression
from sklearn.metrics import classification_report
import requests

def fetch_thingspeak_data(channel_id, api_key):
    """Fetch training data from ThingSpeak"""
    url = f"https://api.thingspeak.com/channels/{channel_id}/feeds.json"
    params = {'api_key': api_key}
    response = requests.get(url, params=params)
    data = response.json()
    
    # Convert to DataFrame
    df = pd.DataFrame(data['feeds'])
    return df

def train_model(df):
    """Train logistic regression model"""
    X = df[['x', 'y', 'z']]
    y = df['is_collision']
    
    X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)
    
    model = LogisticRegression()
    model.fit(X_train, y_train)
    
    # Print model performance
    y_pred = model.predict(X_test)
    print(classification_report(y_test, y_pred))
    
    return model

def export_model_coefficients(model):
    """Export model coefficients for ESP32"""
    coef = model.coef_[0]
    bias = model.intercept_[0]
    
    print("// Update these values in the ESP32 code")
    print("float modelWeights[3] = {", end="")
    print(f"{coef[0]:.6f}, {coef[1]:.6f}, {coef[2]:.6f}")
    print("};")
    print(f"float modelBias = {bias:.6f};")

if __name__ == "__main__":
    # Replace with your ThingSpeak details
    CHANNEL_ID = "YOUR_CHANNEL_ID"
    API_KEY = "YOUR_API_KEY"
    
    # Fetch and prepare data
    df = fetch_thingspeak_data(CHANNEL_ID, API_KEY)
    
    # Train model
    model = train_model(df)
    
    # Export coefficients
    export_model_coefficients(model)