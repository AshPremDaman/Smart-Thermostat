import matplotlib.pyplot as plt
from matplotlib.widgets import Slider
import random

def calc_measured(true_vals):

    measured_vals = []

    for tv in true_vals:
        measured_vals.append(tv + random.uniform(-0.36, 0.36))

    return measured_vals

def calc_avg(measured_vals, num_of_samples):
    avg = []
    index = 0
    samples = []
    for x in range(num_of_samples):
        samples.append(measured_vals[0])
    total = measured_vals[0] * num_of_samples

    for m in measured_vals:
        old_t = samples[index]
        samples[index] = m
        index += 1
        total += m
        total -= old_t
        avg.append(total / num_of_samples)

        if index >= num_of_samples:
            index = 0
    
    return avg

def kalman_filter(measured_vals, q):
    # estimated values array
    estimated = []
    # standatd deviation of measurements = 0.2 C
    omega = 0.36 ** 2
    # kalman estimate
    K = 1.0
    # 1st guess at temperature
    x = 70.0
    # estimate uncertainty p
    p = x ** 4
    print(p)
    for m in measured_vals:
        p = p + q

        K = p / (p + omega)
        x = x + K * (m - x)
        p = (1 - K) * p

        estimated.append(x)

    return estimated

measured_vals = []
with open("data/data_3.txt", 'r') as inputfile:
    for line in inputfile.readlines():
        measured_vals.append(float(line))

estimated = kalman_filter(measured_vals, 0.0003)
avgs = calc_avg(measured_vals, 4)


fig = plt.figure()
plt.subplots_adjust(bottom=0.25)
ax = fig.subplots()

p, = ax.plot(measured_vals, 'b')
p, = ax.plot(estimated, 'r')
pp, = ax.plot(avgs, 'y')

avg_slide = plt.axes([0.25, 0.1, 0.65, 0.03])
k_slide = plt.axes([0.25, 0.05, 0.65, 0.03])
 
avg_factor = Slider(avg_slide, 'Avg Window',
                  1, 20, valinit=2, valstep=1)
k_factor = Slider(k_slide, 'Kalman Noise Factor',
                  0.00000001, 0.005, valinit=0.0003, valstep=0.00000005)

def update_avg(w):
    avgs = calc_avg(measured_vals, int(w))
    pp.set_ydata(avgs)
    fig.canvas.draw()

def update_kalman(q):
    estimated = kalman_filter(measured_vals, q)
    p.set_ydata(estimated)
    fig.canvas.draw()

avg_factor.on_changed(update_avg)
k_factor.on_changed(update_kalman)


plt.xlabel("Time")
plt.ylabel("Temperature")
plt.legend(['Measured', 'Estimates', 'Average'])
plt.show()