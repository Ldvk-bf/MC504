import random
import threading
import time


class RollerCoaster:
    def __init__(self, capacity):
        self.capacity = capacity
        self.boarded = 0
        self.unboarded = 0
        self.board_mutex = threading.Semaphore(1)
        
        self.board_queue = threading.Semaphore(0)
        self.unboard_queue = threading.Semaphore(0)
        
        self.allAboard = threading.Semaphore(0)
        self.allAshore = threading.Semaphore(0)

    def passenger(self, passenger_id):
        print(f"Passenger {passenger_id} is waiting to board.")

        
        self.board_queue.acquire()
        self.board(passenger_id)

        self.unboard_queue.acquire()
        self.unboard(passenger_id)

    def board(self, passenger_id):
        self.board_mutex.acquire()
        self.boarded += 1
        print(f"Passenger {passenger_id} boarded. ({self.boarded}/{self.capacity})")
        print("🎢" + "🚶" * self.boarded + " _" * (4 - self.boarded) + "🎢")
        time.sleep(random.uniform(0.5, 1.5))  # Simulate boarding time
        if self.boarded == self.capacity:
            print("Roller coaster is full. Starting the ride!")
            self.allAboard.release()
        self.board_mutex.release()

    def unboard(self, passenger_id):
        self.board_mutex.acquire()
        self.boarded -= 1
        time.sleep(random.uniform(0.5, 1.5))  # Simulate unboarding time
        print("🎢" + "_ " * (4 - self.boarded) + "🚶" * self.boarded + "🎢")
        print(f"Passenger {passenger_id} unboarded. ({self.capacity - self.boarded}/{self.capacity})")
        if self.boarded == 0:
            print("All passengers have unboarded. Ready for the next ride!")
            self.allAshore.release()
            self.board_mutex.release()
        else:
            self.board_mutex.release()

    def roller_coaster(self):
        while True:
            print("Roller coaster is ready for boarding.")
            
            # Deixa o pessoal entrar
            self.board_queue.release(self.capacity)
            # Espera o pessoal entrar
            self.allAboard.acquire()
            
            print("Roller coaster is running...")
            time.sleep(2)  # Simulate the ride
            print("Roller coaster ride finished.")

            # Deixa o pessoal sair
            self.unboard_queue.release(self.capacity)
            # Espera o pessoal sair
            self.allAshore.acquire()

def shutdown_program():
    time.sleep(10)
    print("Shutting down the program after 10 seconds.")
    exit()

if __name__ == "__main__":
    capacity = 4
    num_passengers = 12
    roller_coaster = RollerCoaster(capacity)

    threads = []
    for i in range(num_passengers):
        t = threading.Thread(target=roller_coaster.passenger, args=(i,))
        threads.append(t)
        t.start()

    roller_coaster_thread = threading.Thread(target=roller_coaster.roller_coaster)
    roller_coaster_thread.start()

    shutdown_thread = threading.Thread(target=shutdown_program)
    shutdown_thread.start()
    
    for t in threads:
        t.join()
    roller_coaster_thread.join()
    shutdown_thread.join()