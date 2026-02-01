import os
import numpy as np
import cv2
import json
# flaskモジュールをインポート
from flask import Flask, render_template, request, jsonify
import google.generativeai as genai
from PIL import Image

app = Flask(__name__, static_folder='resource')

with open("config.json", "r") as file:
    data = json.load(file)
    TOKEN = data["token"]

genai.configure(api_key=TOKEN)
model = genai.GenerativeModel('gemini-3.0-flash')

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

    if img is None: #画像imgの読み込みが失敗したら
        print("Image file is Nothing.")
        return jsonify({"error": "No image"}), 400
    
    #画像をPILに変換
    img_rgb = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
    img_pil = Image.fromarray(img_rgb)

    prompt = """
    この画像を分析し、映っているハンドサインを以下のルールで分類して、
    該当する「数字のみ」を返してください。余計な文字は一切含めないでください。

    1: OKサイン (親指と人差指で輪を作る)
    2: 手のひらを見せる(じゃんけんの「パー」の形)
    3: 人差し指だけを上に立てている
    4: 人差し指だけを下に向けている
    0: それ以外、または手が写っていない、判別不能
    """

    try:
        #Geminiに画像と命令を投げる
        response = model.generate_content([prompt, img_pil])
        
        result_text = response.text.strip()
        
        print(f"Gemini Answer: {result_text}")

        #結果をJSONで返す
        return jsonify({
            "command": int(result_text), # 0~4の数字
            "raw_text": result_text      # 確認用
        })

    except Exception as e:
        print(f"Error: {e}")
        return jsonify({"command": 0, "error": str(e)})

# メイン関数のおまじない  
if __name__ == "__main__": 
    app.run(host="0.0.0.0", debug=True)
