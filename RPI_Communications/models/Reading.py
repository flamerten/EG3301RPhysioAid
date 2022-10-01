import json

class Reading:
    def __init__(self, **kwargs):
        for k in kwargs.keys():
            self.__setattr__(k, kwargs[k])
    
    def to_json(self):
        return json.dumps(self.__dict__)
