const express = require('express')
const mysql = require('mysql')
const bodyParser = require('body-parser')

const db = mysql.createConnection({
        host: 'localhost',
        user: 'root',
        password: '123',
        database: 'test'
})
db.connect(err => {
        if (err) throw err
        console.log('Backend connected to MySQL database.')
})

const app = express()
app.use(bodyParser.json())

app.post('/login', (req, res) => {
        const { username, password } = req.body
        const query = "SELECT * FROM users WHERE login = '" + username + "' AND password = '" + password + "'"
        db.query(query, (err, result) => {
                if (err) {
                        console.error(err)
                        res.json({ success: false, errorMessage: err.message })
                        return
                }
                if (result) res.json({ success: result.length > 0, infos: result })
        })
})
app.get('/login', (req, res) => {
        const { username, password } = req.query
        const query = "SELECT * FROM users WHERE login = '" + username + "' AND password = '" + password + "'"
        db.query(query, (err, result) => {
                if (err) {
                        console.error(err)
                        res.json({ success: false, errorMessage: err.message })
                        return
                }
                if (result) res.json({ success: result.length > 0, infos: result })
        })
})
app.listen(8282, () => { console.log('MySQL backend listen on port 8282.') })
