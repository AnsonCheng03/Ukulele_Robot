# loop_manager.py
import asyncio
import threading

# Create the loop
global_asyncio_loop = asyncio.new_event_loop()

def start_background_loop():
    asyncio.set_event_loop(global_asyncio_loop)
    print("✅ AsyncIO background loop started")
    global_asyncio_loop.run_forever()

# Start it in background thread
threading.Thread(target=start_background_loop, daemon=True).start()
