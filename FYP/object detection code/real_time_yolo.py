import torch
import numpy as np
import cv2
import pafy
import time


class ObjectDetection:
    """
    Class implements Yolo5 model to make inferences on a youtube video using OpenCV.
    """
    
    def __init__(self):
        """
        Initializes the class with youtube url and output file.
        :param url: Has to be as youtube URL,on which prediction is made.
        :param out_file: A valid output file name.
        """
        self.model = self.load_model()
        self.classes = self.model.names
        self.device = 'cuda' if torch.cuda.is_available() else 'cpu'
        print("\n\nDevice Used:",self.device)



    def load_model(self):
        """
        Loads Yolo5 model from pytorch hub.
        :return: Trained Pytorch model.
        """
        model = torch.hub.load('ultralytics/yolov5', 'yolov5l', pretrained=True)
        return model


    def score_frame(self, frame):
        """
        Takes a single frame as input, and scores the frame using yolo5 model.
        :param frame: input frame in numpy/list/tuple format.
        :return: Labels and Coordinates of objects detected by model in the frame.
        """
        self.model.to(self.device)
        frame = [frame]
        results = self.model(frame)
     
        labels, cord = results.xyxyn[0][:, -1], results.xyxyn[0][:, :-1]
        return labels, cord
    
    
    def class_to_label(self, x):
        """
        For a given label value, return corresponding string label.
        :param x: numeric label
        :return: corresponding string label
        """
        return self.classes[int(x)]


    def plot_boxes(self, results, frame):
        """
        Takes a frame and its results as input, and plots the bounding boxes and label on to the frame.
        :param results: contains labels and coordinates predicted by model on the given frame.
        :param frame: Frame which has been scored.
        :return: Frame with bounding boxes and labels ploted on it.
        """
        labels, cord = results
        n = len(labels)
        x_shape, y_shape = frame.shape[1], frame.shape[0]
        for i in range(n):
            row = cord[i]
            if row[4] >= 0.2:
                x1, y1, x2, y2 = int(row[0]*x_shape), int(row[1]*y_shape), int(row[2]*x_shape), int(row[3]*y_shape)
                bgr = (0, 255, 0)
                cv2.rectangle(frame, (x1, y1), (x2, y2), bgr, 2)
                cv2.putText(frame, self.class_to_label(labels[i]), (x1, y1), cv2.FONT_HERSHEY_SIMPLEX, 0.9, bgr, 2)

        return frame


    def __call__(self):
        """
        This function is called when class is executed, it runs the loop to read the video frame by frame,
        and write the output into a new file.
        :return: void
        """
        cap = cv2.VideoCapture(0)

        while cap.isOpened():
            
            start_time = time.perf_counter()
            ret, frame = cap.read()
            if not ret:
                break
            results = self.score_frame(frame)
            frame = self.plot_boxes(results, frame)
            end_time = time.perf_counter()
            fps = 1 / np.round(end_time - start_time, 3)
            cv2.putText(frame, f'FPS: {int(fps)}', (20,70), cv2.FONT_HERSHEY_SIMPLEX, 1.5, (0,255,0), 2)
            cv2.imshow("img", frame)
            print(results)

            if cv2.waitKey(1) & 0xFF == ord('q'):
                break


# Create a new object and execute.
detection = ObjectDetection()
detection()


# # #######################################################################


# # The bounding box values you see are in the format [x_min, y_min, x_max, y_max, confidence]. Here's an explanation of each value:

# # x_min: The x-coordinate of the top-left corner of the bounding box.
# # y_min: The y-coordinate of the top-left corner of the bounding box.
# # x_max: The x-coordinate of the bottom-right corner of the bounding box.
# # y_max: The y-coordinate of the bottom-right corner of the bounding box.
# # confidence: The confidence score associated with the detected object.

# ######################################################################################

# import torch
# import numpy as np
# import cv2
# import pafy
# import time
# from ultralytics import YOLO

# class ObjectDetection:
#     def __init__(self):
#         self.model = self.load_model()
#         self.classes = self.model.names
#         self.device = 'cuda' if torch.cuda.is_available() else 'cpu'
#         print("\n\nDevice Used:", self.device)

#     def load_model(self):
#         model = YOLO('yolov8n.pt')# torch.hub.load('ultralytics/yolov8', 'yolov8n', pretrained=True,trust_repo=True)
#         return model
    
#     def score_frame(self, frame):
#         self.model.to(self.device)
#         frame = [frame]
#         results = self.model(frame)

#         if isinstance(results[0], list):
#             # Handling the case where results is a list
#             labels, cord, confidence = results[0][:, -1], results[0][:, :-1], results[0][:, -2]
#         else:
#             # Handling the case where results is not a list
#             labels, cord, confidence = results[:, -1], results[:, :-1], results[:, -2]

#         return labels, cord, confidence
    
#     def class_to_label(self, x):
#         return self.classes[int(x)]

#     def print_detections(self, results):
#         labels, cord, confidence = results
#         n = min(len(labels), 3)  # At most 2 detections

#         for i in range(n):
#             row = cord[i]
#             if confidence[i] >= 0.2:
#                 label_text = f"Class: {self.class_to_label(labels[i])}, class_id: {labels[i]}, Confidence: {confidence[i]:.2f}, Bounding Box: {row}"
#                 print(label_text)

#     def plot_boxes(self, results, frame):
#         labels, cord, confidence = results
#         n = min(len(labels), 2)  # At most 2 detections
#         x_shape, y_shape = frame.shape[1], frame.shape[0]

#         for i in range(n):
#             row = cord[i]
#             if confidence[i] >= 0.2:
#                 x1, y1, x2, y2 = int(row[0] * x_shape), int(row[1] * y_shape), int(row[2] * x_shape), int(row[3] * y_shape)
#                 bgr = (0, 255, 0)
#                 cv2.rectangle(frame, (x1, y1), (x2, y2), bgr, 2)
#                 label_text = f"{self.class_to_label(labels[i])} {confidence[i]:.2f}"
#                 cv2.putText(frame, label_text, (x1, y1), cv2.FONT_HERSHEY_SIMPLEX, 0.9, bgr, 2)

#         return frame

#     def __call__(self):
#         cap = cv2.VideoCapture(0)

#         while cap.isOpened():
#             start_time = time.perf_counter()
#             ret, frame = cap.read()
#             if not ret:
#                 break

#             results = self.score_frame(frame)
#             self.print_detections(results)  # Print detections
#             frame = self.plot_boxes(results, frame)
#             end_time = time.perf_counter()
#             fps = 1 / np.round(end_time - start_time, 3)
#             cv2.putText(frame, f'FPS: {int(fps)}', (20, 70), cv2.FONT_HERSHEY_SIMPLEX, 1.5, (0, 255, 0), 2)
#             cv2.imshow("img", frame)

#             if cv2.waitKey(1) & 0xFF == ord('q'):
#                 break


# # Create a new object and execute.
# detection = ObjectDetection()
# detection()

