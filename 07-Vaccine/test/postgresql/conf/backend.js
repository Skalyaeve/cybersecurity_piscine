const express = require('express')
const { Client } = require('pg')
const bodyParser = require('body-parser')

const db = new Client({
        host: 'localhost',
        user: 'postgres',
        password: '123',
        database: 'test'
})
db.connect(err => {
        if (err) throw err
        console.log('Backend connected to PostgreSQL database.')
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
                if (result && result.rows)
                        res.json({ success: result.rows.length > 0, result: result.rows })
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
                if (result) res.json({ success: result.rows.length > 0, result: result.rows })
        })
})
app.listen(8383, () => { console.log('PostgreSQL backend listen on port 8383.') })
