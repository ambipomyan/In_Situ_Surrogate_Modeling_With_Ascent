import numpy as np
import pickle
import os

STATE_FILE = "state.pkl"

def load_state():
    if os.path.exists(STATE_FILE):
        with open(STATE_FILE, "rb") as f:
            return pickle.load(f)
    return [1.0, 0.0, 0.0, 0.0, 0.0], 0.0, [0.1, 0.1, 0.1, 0.1, 0.1]  # Initial values

def save_state(X, y, w):
    with open(STATE_FILE, "wb") as f:
        pickle.dump((X, y, w), f)

def update():
    X, y, w = load_state()

    mesh = ascent_data().child(0)
    e_vals = mesh["fields/velocity/values/u"]
    n_iter = mesh["state/cycle"]
    e_max = e_vals.max()

    # First 4 iterations
    if n_iter < 5:
        X[n_iter] = e_max
    # The 5th iteration
    elif n_iter == 5:
        y = e_max
    # Starting from 6th iteration
    else:
        X[0] = 1.0
        X[1:-1] = X[2:]
        X[-1] = y
        y = e_max

        # update w with SGD
        lr = 0.0000001
        epochs = 100

        for epoch in range(epochs):
            # Perform predictions and compute gradients
            pred = np.dot(w, X)

            loss_grad = pred - y

            w_grad = [x * loss_grad for x in X]

            # Update weights using the gradient
            g = [lr * x for x in w_grad]
            w = [a - b for a, b in zip(w, g)]

    # Compute loss for this epoch
    loss = (y - np.dot(w, X))**2 / n_iter

    print(f"Iteration {n_iter}: X = {X}, y = {y}, w = {w}, loss = {loss}")

    save_state(X, y, w)

    return X, y, w

if __name__ == "__main__":
    update()
