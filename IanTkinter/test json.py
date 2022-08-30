from pathlib import Path
import matplotlib
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure
import matplotlib.animation as animation
from matplotlib import style

import tkinter as tk
from tkinter import ttk, Tk, Canvas, Entry, Text, Button

import json
import random

matplotlib.use("TkAgg")
style.use("seaborn-whitegrid")

fig1 = Figure(figsize=(5,5), dpi=100, facecolor="#D9D9D9") #big emg graph for p1
P1_overview_angle = fig1.add_subplot(111)

with open('angle_json_data.json','r') as json_file1:
        angle_data = json.load(json_file1)

angle = [int(angle_data['physio_data'][0]['Patient1']), int(angle_data['physio_data'][1]['Patient2']), int(angle_data['physio_data'][2]['Patient3'])]


people_angle_data = {} #take note a dictionary is used for multiple data for both people and timings
people_emg_data = {}
patient_names = ["Patient1","Patient2","Patient3"]
timings = {}

for pt in patient_names:
    people_angle_data[pt] = [0] #initalise as empty lists with number zero
    people_emg_data[pt] = [0]
    timings[pt] = [0]


def update_records(i):
    global people_angle_data
    global people_emg_data
    global timings

    with open('angle_json_data.json','r') as json_file1:
        angle_data = json.load(json_file1)

    angle = [int(angle_data['physio_data'][0]['Patient1']), int(angle_data['physio_data'][1]['Patient2']), int(angle_data['physio_data'][2]['Patient3'])]

    people_angle_data["Patient1"].append(angle[0])
    timings["Patient1"].append(timings["Patient1"][-1] + 0.125)

animate_func1 = lambda i: a_animateshort("Patient1",P1_overview_angle)


print(people_angle_data, people_emg_data,timings)
print(people_angle_data["Patient1"])
print(timings["Patient1"])

def a_animateshort(patient_no,fig_object):
    update_records(patient_no)
    global people_angle_data
    global timings
    
    xList1 = timings[patient_no]
    yList1 = people_angle_data[patient_no]

    print("xList1",xList1)
    print("yList", yList1)

    fig_object.clear()
    fig_object.set_ylim(0,200)
    fig_object.plot(xList1[-100:-1], yList1[-100:-1])

class PhysioCmdr(tk.Tk):

    def __init__(self, *args, **kwargs):
        tk.Tk.__init__(self, *args, **kwargs)
        tk.Tk.wm_title(self, "PhysioCmdr")
        tk.Tk.iconbitmap(self, default="clienticon.ico")

        container = tk.Frame(self)
        container.pack(side="top", fill="both", expand = True)
        container.grid_rowconfigure(0, weight=1)
        container.grid_columnconfigure(0, weight=1)

        self.frames = {}

        for F in (DetailPage1, ):

            frame = F(container, self)

            self.frames[F] = frame

            frame.grid(row=0, column=0, sticky="nsew")

        self.show_frame(DetailPage1)

    def show_frame(self, cont):

        frame = self.frames[cont]
        frame.tkraise()


class DetailPage1(tk.Frame):

    def __init__(self, parent, controller):
        tk.Frame.__init__(self,parent)
        
        self.initUI()

        anglegraph_p1 = FigureCanvasTkAgg(fig1, self)
        anglegraph_p1.draw()
        anglegraph_p1.get_tk_widget().place(x=47, y=256, width=780, height=200)

    def initUI(self):
        canvas = Canvas(self, height = 768, width = 1024,)
        canvas.place(x = 0, y = 0)

        #Top Banner
        canvas.create_rectangle(0.0, 0.0, 1024.0, 38.0, fill="#0058DD", outline="") 
        #Text in Top Banner
        canvas.create_text(512.0, 19.0, anchor="center", text="PhysioCmdr - Details", fill="#FFFFFF", font=("Inter", 17 * -1))
        #Background for detail
        canvas.create_rectangle(27.0, 88.0, 997.0, 718.0, fill="#D9D9D9", outline="")
        #Name of Patient            
        canvas.create_text(47.0, 108.0, anchor="nw", text="Patient 1", fill="#000000", font=("Inter", 28 * -1))
        #Text for age
        canvas.create_text(47.0, 143.0, anchor="nw", text="Age: 42", fill="#000000", font=("Inter", 15 * -1))
        #Text for last visit
        canvas.create_text(47.0, 161.0, anchor="nw", text="Last Visit: 28/08/2022", fill="#000000", font=("Inter", 15 * -1))
        #Photo of patient    
        canvas.create_rectangle(887.0, 108.0, 977.0, 228.0, fill="#FF0000", outline="")
        #Text above first graph
        canvas.create_text(47.0, 235.0, anchor="nw", text="Knee Angle", fill="#000000", font=("Inter", 17 * -1))
        #Text above second graph  
        canvas.create_text(47.0, 476.0, anchor="nw", text="Muscle Activation", fill="#000000", font=("Inter", 17 * -1))
        #Live text for current first graph value
        canvas.create_text(960.0, 327.0, anchor="ne", text="80"+"\u00B0", fill="#000000", font=("Inter", 48 * -1))
        #Live text for current second graph value
        canvas.create_text(980.0, 568.0, anchor="ne", text="14.5"+"%", fill="#000000", font=("Inter", 48 * -1))

app = PhysioCmdr()
app.geometry("1024x768")

UPDinterval = 1000
ani1 = animation.FuncAnimation(fig1, animate_func1, interval=UPDinterval)

app.mainloop()
