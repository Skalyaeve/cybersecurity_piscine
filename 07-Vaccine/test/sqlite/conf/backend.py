from flask import Flask, jsonify, request
import sqlite3


app = Flask(__name__)

@app.route("/login", methods=["POST", "GET"])
def login():
    if request.method == "POST":
        data = request.json
        username = data["username"]
        password = data["password"]
    elif request.method == "GET":
        username = request.args.get("username")
        password = request.args.get("password")

    conn = sqlite3.connect("test.db")
    cursor = conn.cursor()
    try:
        query = "SELECT * FROM users WHERE login = '" + username + "' AND password = '" + password + "'"
        cursor.execute(query)
        users = cursor.fetchall()
        success = len(users) > 0
        return jsonify({"success": success, "infos": users})
    except Exception as e:
        return jsonify({"success": False, "message": str(e)})
    finally:
        conn.close()


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=8484)