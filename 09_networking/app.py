from flask import Flask, request, jsonify
from openai import OpenAI
import os

# To run, export the openai api key and then python app.py

app = Flask(__name__)
client = OpenAI(api_key=os.environ["OPENAI_API_KEY"])

@app.route("/generate-haiku", methods=["POST"])
def generate_haiku():
    data = request.get_json()
    fact = data.get("fact", "")

    prompt = f"""
Write a haiku inspired by this dog fact:

{fact}
"""

    response = client.chat.completions.create(
        model="gpt-4.1-mini",
        messages=[
            {"role": "system", "content": "You are a poetic assistant."},
            {"role": "user", "content": prompt}
        ]
    )

    haiku = response.choices[0].message.content

    return jsonify({
        "haiku": haiku
    })

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5050, debug=True)

