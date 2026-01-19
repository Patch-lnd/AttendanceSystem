const db = require('../db')
const checkCard = (req,res)=>{
    const {uid} = req.body;
    
    const query = 'SELECT * FRPM users WHERE id = ?';
    db.query(query, [uid], (err, result)=>{
        if(err){
            res.status(500).json({message: 'Erreur serveur'});
        }
        if (result.length > 0) {
            res.status(200).json({message: 'UID trouvé', user: result[0]})
        }else{
            res.status(404).json({message: 'UID non trouvé'})
        }
    })
};
module.exports = {checkCard};


