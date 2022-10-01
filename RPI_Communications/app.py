from flask import Flask, render_template
from flask_socketio import SocketIO, emit

import json
import constants

app = Flask(__name__)
socketio = SocketIO(app)

@socketio.on('connect')
def handle_connect():
    print('Connected')

@socketio.on('disconnect')
def handle_disconnect():
    print('Disconnected')

@socketio.on('start')
def handle_start():
    print('Start')

@socketio.on(constants.DEFAULT_READING_EMIT_EVENT)
def handle_log(data):
    print(f'{constants.DEFAULT_READING_EMIT_EVENT}: ' + str(data))
    emit("log", dict(json.loads(data)), broadcast=True)

@app.route("/", methods=['GET', 'POST'])
def home_page():
    return json.dumps("Welcome!") 

if __name__ == '__main__':
    socketio.run(app, host='0.0.0.0', port=5000, debug=True, log_output=True)