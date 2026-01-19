var express = require('express');
var router = express.Router();

/* GET home page. */
router.get('/', (req, res)=> {
  res.render('index', { title: 'Page d\'Acceuil' });
});
router.get('/transactions', (req, res)=>{
  res.render('transactions', {message: null})
})

module.exports = router;
