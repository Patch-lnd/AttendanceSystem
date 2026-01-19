const mysql = require('mysql')
require('dotenv').config()

const db = mysql.createConnection({
    host: process.env.HOST || 'localhost',
    user: process.env.USER || 'root',
    password: process.env.PASSWORD || '',
    database: process.env.DB || 'Attendance'
})
db.connect((err)=>{
    if(err){
        console.log('Erreur de connexion MYSQL: ',err);
        return;
    }
    console.log('MYSQL Conecté !');
})
module.exports = db;