import pandas as pd
import numpy as np
from sklearn.model_selection import train_test_split
from sklearn.ensemble import RandomForestClassifier
from sklearn.metrics import accuracy_score, classification_report
import joblib

# Load datasets
csv_path_no_collision = "path_to_no_collision_file.csv"  # Replace with actual path
csv_path_collision = "path_to_collision_file.csv"        # Replace with actual path

df_no_collision = pd.read_csv(csv_path_no_collision)
df_collision = pd.read_csv(csv_path_collision)

# Combine datasets
df = pd.concat([df_no_collision, df_collision], ignore_index=True)

# Feature engineering: Extract statistical features for each batch_id
def extract_features(group):
    features = {
        "x_mean": group["x"].mean(),
        "x_std": group["x"].std(),
        "x_min": group["x"].min(),
        "x_max": group["x"].max(),
        "y_mean": group["y"].mean(),
        "y_std": group["y"].std(),
        "y_min": group["y"].min(),
        "y_max": group["y"].max(),
        "z_mean": group["z"].mean(),
        "z_std": group["z"].std(),
        "z_min": group["z"].min(),
        "z_max": group["z"].max(),
    }
    return pd.Series(features)

# Apply feature extraction
features = df.groupby("batch_id").apply(extract_features).reset_index()
labels = df.groupby("batch_id")["is_collision"].first().reset_index(name="is_collision")

# Merge features with labels
data = pd.merge(features, labels, on="batch_id")

# Split data into train and test sets
X = data.drop(columns=["batch_id", "is_collision"])
y = data["is_collision"]
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

# Train a Random Forest Classifier
clf = RandomForestClassifier(random_state=42)
clf.fit(X_train, y_train)

# Export trained model
joblib.dump(clf, "collision_clf_model.pkl")

# Evaluate the model
y_pred = clf.predict(X_test)
print("Accuracy:", accuracy_score(y_test, y_pred))
print("Classification Report:\n", classification_report(y_test, y_pred))

# Example prediction for a new batch
new_batch = pd.DataFrame({
    "x": [-2.04, -2.12, -1.96, -2.08, -2.24],
    "y": [0.75, 0.86, 0.67, 0.39, 0.51],
    "z": [-9.06, -9.34, -8.98, -8.90, -9.02]
})
new_features = extract_features(new_batch)
print("Prediction for new batch:", clf.predict([new_features]))