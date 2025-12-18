from kafka import KafkaConsumer
import json
import time
import socket
import os

worker_id = socket.gethostname()
print(f"🚀 Worker {worker_id} запущен")

consumer = None
while consumer is None:
    try:
        consumer = KafkaConsumer(
            'bookings',
            bootstrap_servers='kafka:9092',
            group_id='booking-workers', 
            value_deserializer=lambda x: json.loads(x.decode('utf-8')),
            auto_offset_reset='earliest'
        )
        print(f"Worker {worker_id} подключился к Kafka")
    except Exception as e:
        print(f"Worker {worker_id} ждет Kafka... ({e})")
        time.sleep(3)

print(f"Worker {worker_id} слушает очередь ...")

for message in consumer:
    booking = message.value
    
    print(f"\n{'='*50}")
    print(f"   Worker {worker_id} получил задание:")
    print(f"   Бронирование: {booking['booking_id']}")
    print(f"   Рейс: {booking['flight']}")
    print(f"   Цена: {booking['price']} руб.")
    
    print(f"Обрабатываю...")
    time.sleep(5) 
    
    print(f"Готово! Билет {booking['booking_id']} забронирован")
    print(f"{'='*50}\n")