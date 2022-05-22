from tkinter import *
from tkinter import ttk

from serial import Serial
from serial.tools import list_ports

from motor_control import motor_control

NUM_MOTORS = 6
freqs = [1, 30, 100, 200, 10, 1]
dutys = [0.0, 0.0, 0.0, 0.19, 0.0, 0.0]
delays = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
control = motor_control(NUM_MOTORS, None)

def load_com_ports(com_combo):
    ports = list_ports.comports()
    com_names = [p.device for p in ports]
    com_combo['values'] = com_names
    if len(com_names):
        com_combo.set(com_names[0])
        com_descrip.set(ports[0].description)
        cnt_button['state'] = NORMAL
    else:
        com_combo.set('')
        com_descrip.set('')
        cnt_button['state'] = DISABLED   
    
    return ports

def connect(ports, index):    
    if cnt_button['text'] == 'Connect':
        if(index < 0):
            print('No port detected!')
            return
        control.connect(ports[index].device)
        cnt_button['text'] = 'Disconnect'

    elif cnt_button['text'] == 'Disconnect':
        control.disconnect()
        cnt_button['text'] = 'Connect'

def cmd_start():
    for i in range(NUM_MOTORS):
        freq  = float(freq_entries[i].get())
        duty  = float(duty_entries[i].get())
        delay = float(delay_entries[i].get())
        control.config_motor(i, freq, duty, delay)
    control.start_motors()

def cmd_reload():
    control.stop_motors()
    cmd_start()

root = Tk()
root.title("Valve control")
# root.geometry('100x250')

mainframe = ttk.Frame(root, padding="3 3 12 12")
mainframe.grid(column=0, row=0, sticky=(N, W, E, S))
root.columnconfigure(0, weight=1)
root.rowconfigure(0, weight=1)

ttk.Button(mainframe, text="Reload", command=lambda: load_com_ports(com_combo)).grid(column=1, row=1)
cnt_button = ttk.Button(mainframe, text="Connect", command=lambda: connect(ports, com_combo.current()))
cnt_button.grid(column=2, row=1)

com_descrip = StringVar() 
ttk.Label(mainframe, textvariable=com_descrip).grid(column=2, row=0, columnspan=6)
com_combo = ttk.Combobox(mainframe, width=13)
com_combo.grid(column=1, row=0, sticky=W)
ports = load_com_ports(com_combo)


column = 1
row_init = 2
for i in range(NUM_MOTORS):
    ttk.Label(mainframe, text=f'Valve {i+1}').grid(column=column, row=row_init + i + 1)


column += 1
freq_entries = []
ttk.Label(mainframe, text="Frequency [Hz]").grid(column=column, row=row_init)
for i in range(NUM_MOTORS):
    freq = StringVar()
    freq.set(freqs[i])
    freq_entry = ttk.Entry(mainframe, width=7, textvariable=freq)
    freq_entry.grid(column=column, row=i + row_init + 1, sticky=(W, E))
    freq_entries.append(freq)

column += 1
duty_entries = []
ttk.Label(mainframe, text="Duty cycle [%]").grid(column=column, row=row_init)
for i in range(NUM_MOTORS):
    duty = StringVar()
    duty.set(dutys[i])
    duty_entry = ttk.Entry(mainframe, width=7, textvariable=duty)
    duty_entry.grid(column=column, row=i + row_init + 1, sticky=(W, E))
    duty_entries.append(duty)

column += 1
delay_entries = []
ttk.Label(mainframe, text="Delay [us]").grid(column=column, row=row_init)
for i in range(NUM_MOTORS):
    delay = StringVar()
    delay.set(delays[i])
    delay_entry = ttk.Entry(mainframe, width=7, textvariable=delay)
    delay_entry.grid(column=column, row=i + row_init + 1, sticky=(W, E))
    delay_entries.append(delay) 

ttk.Button(mainframe, text="Reload config", command=cmd_reload).grid(column=1, row=NUM_MOTORS + 3, columnspan=2)
ttk.Button(mainframe, text="Start valves", command=cmd_start).grid(column=2, row=NUM_MOTORS + 3, columnspan=2)
ttk.Button(mainframe, text="Stop valves", command=control.stop_motors).grid(column=3, row=NUM_MOTORS + 3, columnspan=2)

for child in mainframe.winfo_children(): 
    child.grid_configure(padx=5, pady=5)

# feet_entry.focus()
# root.bind("<Return>", calculate)

root.mainloop()
