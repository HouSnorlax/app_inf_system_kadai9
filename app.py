import numpy as np
import cv2
import json
from flask import Flask, render_template, request, jsonify
from google import genai
from google.genai import types
from PIL import Image

app = Flask(__name__, static_folder='resource')

with open("config.json", "r") as file:
    data = json.load(file)
    TOKEN = data["token"]

client = genai.Client(api_key=TOKEN)

UPLOAD_FOLDER = "./uploads"
app.config["UPLOAD_FOLDER"] = UPLOAD_FOLDER


@app.route("/") 
def index():
    return render_template("index.html",title="Test by Flask and OpenCV")

@app.route("/recog",methods=["POST"])
def inference():
    #画像の受け取り
    _bytes = np.frombuffer(request.data, np.uint8)
    img = cv2.imdecode(_bytes, flags=cv2.IMREAD_COLOR)

    if img is None:
        print("Image file is Nothing.")
        return jsonify({"error": "No image"}), 400
    
    #画像をPILに変換
    img_rgb = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
    img_pil = Image.fromarray(img_rgb)

    
    try:
        #Geminiに画像と命令を投げる
        response = client.models.generate_content(
            model='gemini-2.0-flash',
            contents=[
                "この画像を分析し、映っているハンドサインを以下のルールで分類して、"
                "該当する「数字のみ」を返してください。返答はJSON形式で、キーは'command'のみにしてください。"
                "1: OKサイン (親指と人差指で輪を作る)"
                "2: 手のひらを見せる(じゃんけんの「パー」の形)" 
                "3: 人差し指だけを下に向けている"
                "4: 人差し指だけを上に立てている"
                "0: それ以外、または手が写っていない、判別不能",
                img_pil
            ],
            config=types.GenerateContentConfig(
                response_mime_type="application/json",
                response_schema={
                    "type": "OBJECT",
                    "properties": {
                        "command": {"type": "INTEGER"}
                    }
                }
            )
        )
        
        result = response.parsed 
        
        print(f"Result: {result}")
        
        return jsonify(result)

    except Exception as e:
        print(f"Error: {e}")
        return jsonify({"command": 0, "error": str(e)})

if __name__ == "__main__": 
    app.run(host="0.0.0.0", debug=True)
