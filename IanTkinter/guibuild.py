from pathlib import Path
from time import time
import matplotlib
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure
import matplotlib.animation as animation
from matplotlib import style

import tkinter as tk
from tkinter import ttk, Tk, Canvas, Entry, Text, Button

import json, time, os, sys


start_time = time.time()

matplotlib.use("TkAgg")
style.use("seaborn-whitegrid")

#FILE PATHS
icon_file_path = os.path.join(sys.path[0],"AppSource","clienticon.ico")
angle_file_path = os.path.join(sys.path[0],"AppSource","angle_json_data.json") 
semg_file_path = os.path.join(sys.path[0],"AppSource","emg_json_data.json") 

#creating a Figure for each graph im going to plot
fig1 = Figure(figsize=(5,5), dpi=90, facecolor="#D9D9D9") #angle graph for p1
fig2 = Figure(figsize=(5,5), dpi=90, facecolor="#D9D9D9") #emg graph for p1
fig3 = Figure(figsize=(5,5), dpi=90, facecolor="#D9D9D9") #angle graph for p2
fig4 = Figure(figsize=(5,5), dpi=90, facecolor="#D9D9D9") #emg graph for p2
fig5 = Figure(figsize=(5,5), dpi=90, facecolor="#D9D9D9") #angle graph for p3
fig6 = Figure(figsize=(5,5), dpi=90, facecolor="#D9D9D9") #emg graph for p3
fig7 = Figure(figsize=(5,5), dpi=100, facecolor="#D9D9D9") #big angle graph for p1
fig8 = Figure(figsize=(5,5), dpi=100, facecolor="#D9D9D9") #big emg graph for p1
fig9 = Figure(figsize=(5,5), dpi=100, facecolor="#D9D9D9") #big angle graph for p1
fig10 = Figure(figsize=(5,5), dpi=100, facecolor="#D9D9D9") #big emg graph for p1
fig11 = Figure(figsize=(5,5), dpi=100, facecolor="#D9D9D9") #big angle graph for p1
fig12 = Figure(figsize=(5,5), dpi=100, facecolor="#D9D9D9") #big emg graph for p1
P1_overview_angle = fig1.add_subplot(111)
P1_overview_emg = fig2.add_subplot(111)
P2_overview_angle = fig3.add_subplot(111)
P2_overview_emg = fig4.add_subplot(111)
P3_overview_angle = fig5.add_subplot(111)
P3_overview_emg = fig6.add_subplot(111)
P1_detail_angle = fig7.add_subplot(111)
P1_detail_emg = fig8.add_subplot(111)
P2_detail_angle = fig9.add_subplot(111)
P2_detail_emg = fig10.add_subplot(111)
P3_detail_angle = fig11.add_subplot(111)
P3_detail_emg = fig12.add_subplot(111)


people_angle_data = {}
people_emg_data = {}
timings = {}
patient_names = ["Patient1","Patient2","Patient3"]

for pt in patient_names:
    people_angle_data[pt] = [0] #initalise as empty lists with number zero
    people_emg_data[pt] = [0]
    timings[pt] = [0]



def update_records(patient_no):
    global people_angle_data
    global people_emg_data
    global timings
    
    
    try:
        with open(angle_file_path,'r') as json_file1:
            angle_data = json.load(json_file1)

        angle = [int(angle_data['physio_data'][0]['Patient1']), int(angle_data['physio_data'][1]['Patient2']), int(angle_data['physio_data'][2]['Patient3'])]
    except:
        print("unable to get angle from data")
        angle = [0,0,0]
        for i in range(len(patient_names)):
            angle[i] = people_angle_data[patient_names[i]][-1]
    


    with open(semg_file_path,'r') as json_file2:
        emg_data = json.load(json_file2)

    emg = [int(emg_data['physio_data'][0]['Patient1']), int(emg_data['physio_data'][1]['Patient2']), int(emg_data['physio_data'][2]['Patient3'])]     

    for i in range(len(patient_names)):
        people_angle_data[patient_names[i]].append(angle[i])
        people_emg_data[patient_names[i]].append(emg[i])
        timings[patient_names[i]].append(round(time.time()-start_time,2))
    

    people_angle_data[patient_no] = people_angle_data[patient_no][-500:] #last n data
    people_emg_data[patient_no] = people_emg_data[patient_no][-500:] #last n data
    timings[patient_no] = timings[patient_no][-500:]



def a_animateshort(patient_no,fig_object):
    update_records(patient_no)
    global people_angle_data
    global timings
    
    xList1 = timings[patient_no]
    yList1 = people_angle_data[patient_no]

    fig_object.clear()
    fig_object.set_ylim(0,200)
    fig_object.plot(xList1[-100:-1], yList1[-100:-1])

def e_animateshort(patient_no,fig_object):
    update_records(patient_no)
    global people_emg_data
    global timings
    
    xList2 = timings[patient_no]
    yList2 = people_emg_data[patient_no]

    fig_object.clear()
    fig_object.set_ylim(0,100)
    fig_object.plot(xList2[-100:-1], yList2[-100:-1])

def a_animatelong(patient_no,fig_object):
    update_records(patient_no)
    global people_angle_data
    global timings
    
    xList3 = timings[patient_no]
    yList3 = people_angle_data[patient_no]

    fig_object.clear()
    fig_object.set_ylim(0,200)
    fig_object.plot(xList3, yList3)

def e_animatelong(patient_no,fig_object):
    update_records(patient_no)
    global people_emg_data
    global timings
    
    xList4 = timings[patient_no]
    yList4 = people_emg_data[patient_no]

    fig_object.clear()
    fig_object.set_ylim(0,100)
    fig_object.plot(xList4, yList4)

animate_func1 = lambda i: a_animateshort("Patient1",P1_overview_angle) 
animate_func2 = lambda i: e_animateshort("Patient1",P1_overview_emg)
animate_func3 = lambda i: a_animateshort("Patient2",P2_overview_angle) 
animate_func4 = lambda i: e_animateshort("Patient2",P2_overview_emg)
animate_func5 = lambda i: a_animateshort("Patient3",P3_overview_angle) 
animate_func6 = lambda i: e_animateshort("Patient3",P3_overview_emg)
animate_func1_a = lambda i: a_animatelong("Patient1",P1_detail_angle) 
animate_func1_e = lambda i: e_animatelong("Patient1",P1_detail_emg)
animate_func2_a = lambda i: a_animatelong("Patient2",P2_detail_angle) 
animate_func2_e = lambda i: e_animatelong("Patient2",P2_detail_emg)
animate_func3_a = lambda i: a_animatelong("Patient3",P3_detail_angle) 
animate_func3_e = lambda i: e_animatelong("Patient3",P3_detail_emg)

class PhysioCmdr(tk.Tk):

    def __init__(self, *args, **kwargs):
        tk.Tk.__init__(self, *args, **kwargs)
        tk.Tk.wm_title(self, "PhysioCmdr")
        tk.Tk.iconbitmap(self, default=(icon_file_path))

        container = tk.Frame(self)
        container.pack(side="top", fill="both", expand = True)
        container.grid_rowconfigure(0, weight=1)
        container.grid_columnconfigure(0, weight=1)
        self.frames = {}

        for F in (Overview, DetailPage1, DetailPage2, DetailPage3):

            frame = F(container, self)

            self.frames[F] = frame

            frame.grid(row=0, column=0, sticky="nsew")

        self.show_frame(Overview)

    def show_frame(self, cont):

        frame = self.frames[cont]
        frame.tkraise()

class Overview(tk.Frame):

    def __init__(self, parent, controller):
        tk.Frame.__init__(self,parent)
        
        self.initUI()

        button1 = ttk.Button(self,text="Details",
                            command=lambda: controller.show_frame(DetailPage1))
        button1.place(x=51.0, y=184.0, width=120, height=34)

        button2 = ttk.Button(self, text="Details",
                            command=lambda: controller.show_frame(DetailPage2))
        button2.place(x=382.0, y=184.0, width=120, height=34)

        button3 = ttk.Button(self, text="Details",
                            command=lambda: controller.show_frame(DetailPage3))
        button3.place(x=713.0, y=184.0, width=120, height=34)

        p1anglegraph = FigureCanvasTkAgg(fig1, self)
        p1anglegraph.draw()
        p1anglegraph.get_tk_widget().place(x=51.0, y=256.0, width=260, height=200)

        p1emggraph = FigureCanvasTkAgg(fig2, self)
        p1emggraph.draw()
        p1emggraph.get_tk_widget().place(x=51.0, y=497.0, width=260, height=200)

        p2anglegraph = FigureCanvasTkAgg(fig3, self)
        p2anglegraph.draw()
        p2anglegraph.get_tk_widget().place(x=382.0, y=256.0, width=260, height=200)

        p2emggraph = FigureCanvasTkAgg(fig4, self)
        p2emggraph.draw()
        p2emggraph.get_tk_widget().place(x=382.0, y=497.0, width=260, height=200)

        p3anglegraph = FigureCanvasTkAgg(fig5, self)
        p3anglegraph.draw()
        p3anglegraph.get_tk_widget().place(x=713.0, y=256.0, width=260, height=200)

        p3emggraph = FigureCanvasTkAgg(fig6, self)
        p3emggraph.draw()
        p3emggraph.get_tk_widget().place(x=713.0, y=497.0, width=260, height=200)

    def initUI(self):
        canvas = Canvas(self, height = 768, width = 1024,)
        canvas.place(x = 0, y = 0)
        
        #Top Banner
        canvas.create_rectangle(0.0, 0.0, 1024.0, 38.0, fill="#0058DD", outline="") 
        #Text in Top Banner
        canvas.create_text(512.0, 19.0, anchor="center", text="PhysioCmdr - Overview", fill="#FFFFFF", font=("Inter", 17 * -1))
        #Background of Patient 1 UI
        canvas.create_rectangle(31.0, 88.0, 331.0, 718.0, fill="#D9D9D9", outline="")
        #Text above first graph of P1
        canvas.create_text(51.0, 235.0, anchor="nw", text="Knee Angle", fill="#000000", font=("Inter", 17 * -1))
        #Text above second graph of P1
        canvas.create_text(51.0, 476.0, anchor="nw", text="Muscle Activation", fill="#000000", font=("Inter", 17 * -1))
        #Text for name of P1
        canvas.create_text(51.0, 108.0, anchor="nw", text="Patient 1", fill="#000000", font=("Inter", 28 * -1))
        #Photo of P1
        canvas.create_rectangle(221.0, 108.0, 311.0, 228.0, fill="#FF0000", outline="")
        #Background of Patient 2 UI
        canvas.create_rectangle(362.0, 88.0, 662.0, 718.0, fill="#D9D9D9", outline="")
        #Text above first graph of P2
        canvas.create_text(382.0, 235.0, anchor="nw", text="Knee Angle", fill="#000000", font=("Inter", 17 * -1))
        #Text above second graph of P2
        canvas.create_text(382.0, 476.0, anchor="nw", text="Muscle Activation", fill="#000000",font=("Inter", 17 * -1))
        #Text for name of P2
        canvas.create_text(382.0, 108.0, anchor="nw", text="Patient 2", fill="#000000", font=("Inter", 28 * -1))
        #Photo of P2
        canvas.create_rectangle(552.0, 108.0, 642.0, 228.0, fill="#FF0000", outline="")
        #Background of Patient 3 UI
        canvas.create_rectangle(693.0, 88.0, 993.0, 718.0, fill="#D9D9D9", outline="")
        #Text above first graph of P3
        canvas.create_text(713.0, 235.0, anchor="nw", text="Knee Angle", fill="#000000", font=("Inter", 17 * -1))
        #Text above second graph of P3
        canvas.create_text(713.0, 476.0, anchor="nw", text="Muscle Activation", fill="#000000", font=("Inter", 17 * -1))
        #Text for name of P3
        canvas.create_text(713.0, 108.0, anchor="nw", text="Patient 3", fill="#000000", font=("Inter", 28 * -1))
        #Photo of P3
        canvas.create_rectangle(883.0, 108.0, 973.0, 228.0, fill="#FF0000", outline="")

class DetailPage1(tk.Frame):
    global angle
    global emg

    def __init__(self, parent, controller):
        tk.Frame.__init__(self,parent)
        
        self.initUI()

        button1 = ttk.Button(self, text="Back to Overview",
                            command=lambda: controller.show_frame(Overview))
        button1.place(x=4.0, y=5.0, width=120, height=30)

        anglegraph_p1 = FigureCanvasTkAgg(fig7, self)
        anglegraph_p1.draw()
        anglegraph_p1.get_tk_widget().place(x=47, y=256, width=780, height=200)

        emggraph1_p1 = FigureCanvasTkAgg(fig8, self)
        emggraph1_p1.draw()
        emggraph1_p1.get_tk_widget().place(x=47, y=497, width=780, height=200)

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
        canvas.create_text(960.0, 327.0, anchor="ne", text="p"+"\u00B0", fill="#000000", font=("Inter", 48 * -1))
        #Live text for current second graph value
        canvas.create_text(980.0, 568.0, anchor="ne", text="14.5"+"%", fill="#000000", font=("Inter", 48 * -1))

class DetailPage2(tk.Frame):

    def __init__(self, parent, controller):
        tk.Frame.__init__(self,parent)
        
        self.initUI()

        button1 = ttk.Button(self, text="Back to Overview",
                            command=lambda: controller.show_frame(Overview))
        button1.place(x=4.0, y=5.0, width=120, height=30)

        anglegraph_p2 = FigureCanvasTkAgg(fig9, self)
        anglegraph_p2.draw()
        anglegraph_p2.get_tk_widget().place(x=47, y=256, width=780, height=200)

        emggraph1_p2 = FigureCanvasTkAgg(fig10, self)
        emggraph1_p2.draw()
        emggraph1_p2.get_tk_widget().place(x=47, y=497, width=780, height=200)

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
        canvas.create_text(47.0, 108.0, anchor="nw", text="Patient 2", fill="#000000", font=("Inter", 28 * -1))
        #Text for age
        canvas.create_text(47.0, 143.0, anchor="nw", text="Age: 22", fill="#000000", font=("Inter", 15 * -1))
        #Text for last visit
        canvas.create_text(47.0, 161.0, anchor="nw", text="Last Visit: 16/06/2022", fill="#000000", font=("Inter", 15 * -1))
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

class DetailPage3(tk.Frame):

    def __init__(self, parent, controller):
        tk.Frame.__init__(self,parent)
        
        self.initUI()

        button1 = ttk.Button(self, text="Back to Overview",
                            command=lambda: controller.show_frame(Overview))
        button1.place(x=4.0, y=5.0, width=120, height=30)

        anglegraph_p3 = FigureCanvasTkAgg(fig11, self)
        anglegraph_p3.draw()
        anglegraph_p3.get_tk_widget().place(x=47, y=256, width=780, height=200)

        emggraph1_p3 = FigureCanvasTkAgg(fig12, self)
        emggraph1_p3.draw()
        emggraph1_p3.get_tk_widget().place(x=47, y=497, width=780, height=200)

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
        canvas.create_text(47.0, 108.0, anchor="nw", text="Patient 3", fill="#000000", font=("Inter", 28 * -1))
        #Text for age
        canvas.create_text(47.0, 143.0, anchor="nw", text="Age: 35", fill="#000000", font=("Inter", 15 * -1))
        #Text for last visit
        canvas.create_text(47.0, 161.0, anchor="nw", text="Last Visit: 32/02/2022", fill="#000000", font=("Inter", 15 * -1))
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

UPDinterval = 100
#Patient 1 SEMG Data and Joint Angle


ani1 = animation.FuncAnimation(fig1, animate_func1, interval=UPDinterval)
ani2 = animation.FuncAnimation(fig2, animate_func2, interval=UPDinterval)
"""
ani3 = animation.FuncAnimation(fig3, animate_func3, interval=UPDinterval)
ani4 = animation.FuncAnimation(fig4, animate_func4, interval=UPDinterval)
ani5 = animation.FuncAnimation(fig5, animate_func5, interval=UPDinterval)
ani6 = animation.FuncAnimation(fig6, animate_func6, interval=UPDinterval)
"""
ani7 = animation.FuncAnimation(fig7, animate_func1_a, interval=UPDinterval)
ani8 = animation.FuncAnimation(fig8, animate_func1_e, interval=UPDinterval)

"""
ani9 = animation.FuncAnimation(fig9, animate_func2_a, interval=UPDinterval)
ani10 = animation.FuncAnimation(fig10, animate_func2_e, interval=UPDinterval)
ani11 = animation.FuncAnimation(fig11, animate_func3_a, interval=UPDinterval)
ani12 = animation.FuncAnimation(fig12, animate_func3_e, interval=UPDinterval)
"""

#app.resizable(False, False)
app.mainloop()
print("App has finished running")
