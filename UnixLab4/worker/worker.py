from kafka import KafkaConsumer
import json
import time
import socket
import os
import signal
import sys

worker_id = socket.gethostname()
print(f"🚀 Worker {worker_id} запущен")

consumer = None
shutdown_flag = False


def signal_handler(signum, frame):
    global shutdown_flag
    print(f"\nWorker {worker_id} получил сигнал SIGTERM, завершаю работу")
    shutdown_flag = True
    if consumer:
        try:
            consumer.close()
            print(f"Worker {worker_id}: Kafka consumer закрыт")
        except Exception as e:
            print(f"Worker {worker_id}: Ошибка при закрытии consumer: {e}")
    sys.exit(0)


signal.signal(signal.SIGTERM, signal_handler)

while consumer is None and not shutdown_flag:
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

if shutdown_flag:
    sys.exit(0)

print(f"Worker {worker_id} слушает очередь ...")

for message in consumer:
    if shutdown_flag:
        break
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
    if shutdown_flag:
        break

# Закрываем consumer
if consumer:
    try:
        consumer.close()
        print(f"✅ Worker {worker_id}: consumer закрыт")
    except:
        pass
