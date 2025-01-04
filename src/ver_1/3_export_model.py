import tensorflow as tf

# Load trained model
model = joblib.load("collision_model.pkl")

# Convert to TensorFlow Lite
converter = tf.lite.TFLiteConverter.from_saved_model("path_to_saved_model")
tflite_model = converter.convert()

# Save TFLite model
with open("collision_model.tflite", "wb") as f:
    f.write(tflite_model)