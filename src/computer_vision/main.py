# OpenMV Color Tracking  Script

# import required libraries and modules for hardware interfacing, image processing, and timing
from pyb import UART, LED 
import sensor, image, time


# Initialize serial communication (UART)
# UART 3 at 115200 bps for communication.
uart = UART(3, 115200)

# Initialize on-board status indicators
red_led = LED(1)
green_led = LED(2)

# Reset and initialize the camera sensor hardware
sensor.reset()

# Configure pixel format to RGB565 (16-bit per pixel, Red-Green-Blue)
sensor.set_pixformat(sensor.RGB565)

# Configure frame size to QQVGA (Quarter Quarter Video Graphics Array: 160x120 pixels)
sensor.set_framesize(sensor.QQVGA)

# Disable Automatic Gain Control (AGC) to ensure deterministic color evaluation
sensor.set_auto_gain(False)

# Disable Automatic White Balance (AWB) and lock precise digital RGB gains 
# to maintain color consistency across variable environmental illumination
sensor.set_auto_whitebal(False, rgb_gain_db=[62.0, 60.0, 62.0])

# Disable Automatic Exposure Control (AEC) and set static exposure to 25,000 microseconds
sensor.set_auto_exposure(False, exposure_us=25000)

# Allow sensor clock and automatic adjustments to stabilize over a 2000ms window
sensor.skip_frames(time=2000)

# Initialize the frame rate tracking clock
clock = time.clock()


# COLOR THRESHOLDS (CIE L*a*b* COLOR SPACE)
# Red Threshold:
red_threshold = (0, 100, 17, 127, 21, 127)
# Hue boundaries for red chrominance validation
RED_H_MIN = 0
RED_H_MAX = 15

# Green Threshold:
green_threshold = (0, 91, -128, -12, -35, 49)

# Validates whether a detected blob conforms to true red chrominance criteria by converting its centroid pixel to the HSV color space.
def is_true_red(img, blob):
    # Sample the RGB values directly from the centroid coordinates of the blob
    r, g, b = img.get_pixel(blob.cx(), blob.cy())

    # Normalize standard 8-bit integer RGB values to floating-point values between 0.0 and 1.0
    r_n, g_n, b_n = r / 255.0, g / 255.0, b / 255.0
    mx = max(r_n, g_n, b_n)
    mn = min(r_n, g_n, b_n)
    df = mx - mn

    # Compute Hue (H) based on the dominant primary color channel
    if mx == mn:
        h = 0
    elif mx == r_n:
        h = (60 * ((g_n - b_n) / df) + 360) % 360
    elif mx == g_n:
        h = (60 * ((b_n - r_n) / df) + 120) % 360
    elif mx == b_n:
        h = (60 * ((r_n - g_n) / df) + 240) % 360

    # Downscale the standard 360-degree Hue circle to an 8-bit compliant 0-179 range
    h_scaled = int(h / 2) 

    # Evaluate the scaled hue against the predefined upper boundary constraint
    return (h_scaled <= RED_H_MAX)


# =============================== MAIN LOOP ===============================

while True:
    # Update internal frame rate monitor clock
    clock.tick()
    
    # Capture the latest image frame from the sensor
    img = sensor.snapshot()

    # Isolate pixel clusters aligning with defined red and green color matrices
    red_blobs = img.find_blobs([red_threshold], pixels_threshold=100, area_threshold=100)
    green_blobs = img.find_blobs([green_threshold], pixels_threshold=100, area_threshold=100)

    # Initialize list to filter and collect candidate targets
    candidates = []

    # Process and filter detected red blobs
    for blob in red_blobs:
        # Enforce Hue validation and structural spatial filtering (excluding the upper horizon)
        if (is_true_red(img, blob) and blob.cy() > (119 / 6)):
            candidates.append(("2", blob))  # Protocol key "2" designates a validated red target

    # Process and filter detected green blobs
    for blob in green_blobs:
        # Enforce spatial filtering based on the horizon threshold
        if blob.cy() > (119 / 6):
            candidates.append(("1", blob))  # Protocol key "1" designates a validated green target

    # Default State: Ensure status indicators are darkened prior to target processing
    red_led.off()
    green_led.off()

    # Process localized candidates if any have passed primary filtering criteria
    if len(candidates) > 0:

        # Select the dominant target based on the maximum pixel area metric
        color, blob = max(candidates, key=lambda item: item[1].pixels())

        # Extract structural bounding dimensions and centroid coordinates
        x, y, w, h = blob.rect()
        cx, cy = blob.cx(), blob.cy()

        # Target Specific Processing: Red Classification
        if color == "2":
            # Render descriptive user interface graphics on the image buffer
            img.draw_rectangle(blob.rect(), color=(255, 0, 0))
            img.draw_cross(cx, cy, color=(255, 0, 0))
            img.draw_string(x, y, "RED", color=(255, 0, 0))

            # Actuate corresponding hardware visual feedback indicators
            red_led.on()
            green_led.off()

        # Target Specific Processing: Green Classification
        else:
            # Render descriptive user interface graphics on the image buffer
            img.draw_rectangle(blob.rect(), color=(0, 255, 0))
            img.draw_cross(cx, cy, color=(0, 255, 0))
            img.draw_string(x, y, "GREEN", color=(0, 255, 0))

            # Actuate corresponding hardware visual feedback indicators
            green_led.on()
            red_led.off()

        # Transmit comma-separated telemetry payload over the serial interface
        uart.write("{},{},{}\n".format(color, cx, cy))
        
    # Output runtime diagnostics to the serial debugging terminal
    print("FPS:", clock.fps())