from fastapi import FastAPI, HTTPException
from pydantic import BaseModel
import psycopg2
from datetime import datetime
import numpy as np
from sklearn.linear_model import LinearRegression

def get_db_connection():
    return psycopg2.connect(
        host="localhost",
        database="monitoring",
        user="postgres",
        password="postgres"
    )

app = FastAPI()

class EnergyData(BaseModel):
    voltage: float
    current: float
    power: float
    energy_consumed: float

def predict_energy(voltage, current, power):
    try:
        conn = get_db_connection()
        cursor = conn.cursor()
        cursor.execute("SELECT voltage, current, power, energy_consumed FROM energy_data")
        rows = cursor.fetchall()
        cursor.close()
        conn.close()

        X = np.array([(row[0], row[1], row[2]) for row in rows])
        y = np.array([row[3] for row in rows])

        model = LinearRegression()
        model.fit(X, y)

        input_data = np.array([[voltage, current, power]])
        predicted_energy = model.predict(input_data)
        return predicted_energy[0]
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Error in prediction: {str(e)}")

@app.post("/api/energy_data/")
async def save_energy_data(data: EnergyData):
    try:
        conn = get_db_connection()
        cursor = conn.cursor()

        cursor.execute("""
            INSERT INTO energy_data (timestamp, voltage, current, power, energy_consumed)
            VALUES (%s, %s, %s, %s, %s)
        """, (
            datetime.now(),
            data.voltage,
            data.current,
            data.power,
            data.energy_consumed
        ))

        conn.commit()

        predicted_energy = predict_energy(data.voltage, data.current, data.power)

        cursor.execute("""
            INSERT INTO energy_data_predictions (timestamp, voltage, current, power, energy_consumed, predicted_energy_consumed)
            VALUES (%s, %s, %s, %s, %s, %s)
        """, (
            datetime.now(),
            data.voltage,
            data.current,
            data.power,
            data.energy_consumed,
            predicted_energy
        ))

        conn.commit()

        cursor.close()
        conn.close()

        return {"message": "Data stored successfully and prediction made!"}
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))
