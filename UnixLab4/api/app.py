from fastapi import FastAPI, HTTPException
from fastapi.responses import HTMLResponse, FileResponse
from fastapi.staticfiles import StaticFiles
from kafka import KafkaProducer
import json
import uuid
import time
import os

app = FastAPI()

app.mount("/static", StaticFiles(directory="static"), name="static")

# Подключение к Kafka
producer = None
while producer is None:
    try:
        producer = KafkaProducer(
            bootstrap_servers='kafka:9092',
            value_serializer=lambda v: json.dumps(v).encode('utf-8')
        )
        print("Connected to Kafka")
    except:
        print("Waiting for Kafka...")
        time.sleep(2)

@app.get("/", response_class=HTMLResponse)
async def home():
    # Читаем HTML из файла
    with open("static/index.html", "r", encoding="utf-8") as f:
        html_content = f.read()
    return html_content

@app.get("/style.css")
async def get_css():
    return FileResponse("static/style.css")

@app.post("/api/book")
async def book_ticket():
    """Создать бронирование - API endpoint"""
    booking_id = str(uuid.uuid4())[:8].upper()
    
    ticket_data = {
        "booking_id": booking_id,
        "flight": "SU-100 Москва → Нью-Йорк",
        "passenger": "Пассажир",
        "price": 45000,
        "status": "processing",
        "timestamp": time.time()
    }
    
    # Отправляем в Kafka
    producer.send("bookings", value=ticket_data)
    
    return {
        "booking_id": booking_id,
        "flight": ticket_data["flight"],
        "price": ticket_data["price"],
        "status": "processing",
        "message": "Бронирование в обработке"
    }

@app.get("/api/stats")
async def get_stats():
    """Получить статистику системы"""
    return {
        "service": "AviaSales",
        "workers": 2,
        "kafka": "connected",
        "status": "operational"
    }

@app.get("/api/scale/{n}")
async def scale_workers(n: int):
    """Получить команду для масштабирования"""
    if n < 1 or n > 10:
        raise HTTPException(status_code=400, detail="Количество воркеров должно быть от 1 до 10")
    
    return {
        "command": f"docker-compose up -d --scale worker={n}",
        "message": f"Запустить эту команду для масштабирования до {n} воркеров",
        "workers": n
    }

@app.get("/health")
async def health():
    return {"status": "ok", "service": "aviasales-api"}