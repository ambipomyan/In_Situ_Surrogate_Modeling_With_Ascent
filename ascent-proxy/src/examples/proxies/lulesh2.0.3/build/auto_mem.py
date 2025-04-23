import numpy as np
from multiprocessing import shared_memory
import atexit

# Create or attach to shared memory
try:
    shm_X = shared_memory.SharedMemory(name="shm_X", create=True, size=5 * 8)  # 5 doubles (8 bytes each)
    shm_y = shared_memory.SharedMemory(name="shm_y", create=True, size=8)  # 1 double
    shm_w = shared_memory.SharedMemory(name="shm_w", create=True, size=5 * 8)  # 5 doubles (8 bytes each)
    X = np.ndarray((5,), dtype=np.float64, buffer=shm_X.buf)
    y = np.ndarray((1,), dtype=np.float64, buffer=shm_y.buf)
    w = np.ndarray((5,), dtype=np.float64, buffer=shm_w.buf)
    X[:] = [1.0, 0.0, 0.0, 0.0, 0.0]  # Initialize values
    y[0] = 0.0
    w[:] = [0.05, 0.05, 0.05, 0.05, 0.05]
except FileExistsError:
    shm_X = shared_memory.SharedMemory(name="shm_X")
    shm_y = shared_memory.SharedMemory(name="shm_y")
    shm_w = shared_memory.SharedMemory(name="shm_w")
    X = np.ndarray((5,), dtype=np.float64, buffer=shm_X.buf)
    y = np.ndarray((1,), dtype=np.float64, buffer=shm_y.buf)
    w = np.ndarray((5,), dtype=np.float64, buffer=shm_w.buf)

def cleanup():
    shm_X.close()
    shm_y.close()
    shm_w.close()
    shm_X.unlink()
    shm_y.unlink()
    shm_w.unlink()

atexit.register(cleanup)

def update():
    mesh = ascent_data().child(0)
    e_vals = mesh["fields/velocity/values/u"]
    n_iter = mesh["state/cycle"]
    e_max = e_vals.max()

    if n_iter < 5:
        X[n_iter] = e_max
    elif n_iter == 5:
        y[0] = e_max
    else:
        X[0] = 1.0
        X[1:-1] = X[2:]
        X[-1] = y[0]
        y[0] = e_max

        if n_iter < 400:
            # update w with SGD
            lr = 0.0000001
            epochs = 100

            for epoch in range(epochs):
                # Perform predictions and compute gradients
                pred = np.dot(w, X)

                loss_grad = pred - y[0]

                w_grad = X * loss_grad

                # Update weights using the gradient
                w[:] = w[:] - lr * w_grad

                loss = (y[0] - np.dot(w, X))**2 / epochs
                
                if n_iter == 100 or n_iter == 200 or n_iter == 300 or n_iter == 400:
                    print(f"Iteration: {n_iter}, epoch: {epoch}, loss: {loss}")


    # Compute loss for this epoch
    #loss = (y[0] - np.dot(w, X))**2 / n_iter

    #if n_iter % 9 == 0:
    #    print(f"Iteration {n_iter}: {np.dot(w, X)}, {y[0]}, {loss}")

    # reset
    if n_iter == 900:
        cleanup()

if __name__ == "__main__":
    update()
