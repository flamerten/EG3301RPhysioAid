# 3301R-project-eim322

NUS project

# Getting Started

Install required packages using `pip install -r requirements.txt`

# Simulate

- Run `python app.py`
- Run `FREQUENCY=<Desired frequency in hz> NUM_OF_SENSORS=<Number of sensors> SIM_DURATION=<Duration of simulation in seconds> python tests/simulate.py`
  - By default frequency is `2hz`, number of sensors is `4` and duration is `20 seconds`.

# TODO

- Flask development server is not intended to be used in a production deployment. Therefore, intention is to use gunicorn as the WSGI server.
- Database storage of readings in a NoSQL database (eg. MongoDB)
- Explore containerization with Docker before hosting on a hyperscaler(eg. Google Cloud Run)
