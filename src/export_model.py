import tensorflow as tf

model = tf.keras.models.load_model('trained_model/collision_clf_model.pkl')  # Load your model
converter = tf.lite.TFLiteConverter.from_keras_model(model)
tflite_model = converter.convert()

with open('collision_clf_model.tflite', 'wb') as f:
    f.write(tflite_model)
