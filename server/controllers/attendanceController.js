// On importe la connexion à la base de données
// Cela nous permet d’exécuter des requêtes SQL
const db = require("../db");

// Cette fonction sera appelée quand une carte RFID est badgée
// Elle est exportée pour être utilisée par une route
exports.handleAttendance = (req, res) => {
       console.log("Route /api/attendance atteinte");
    console.log(req.body);

    // 1) On récupère l’UID RFID envoyé dans le corps de la requête
    // Exemple attendu : { "rfid_uid": "A1B2C3D4" }
    const { rfid_uid } = req.body;

    // 2) Sécurité minimale :
    // Si aucun UID n’est envoyé, on ne peut rien faire
    if (!rfid_uid) {
        return res.status(400).json({
            status: "error",
            message: "UID RFID manquant"
        });
    }

    // 3) Requête SQL pour chercher un utilisateur avec cet UID
    const findUserQuery = "SELECT * FROM users WHERE rfid_uid = ?";

    // 4) On exécute la requête SQL
    // Le ? est remplacé par la valeur de rfid_uid (protection contre injections SQL)
    db.query(findUserQuery, [rfid_uid], (err, results) => {

        // 5) Si la base de données retourne une erreur
        if (err) {
            return res.status(500).json({
                status: "error",
                message: "Erreur serveur lors de la recherche utilisateur"
            });
        }

        // 6) Si aucun utilisateur n’est trouvé
        // results.length === 0 signifie : carte inconnue
        if (results.length === 0) {
            return res.status(404).json({
                status: "error",
                message: "Utilisateur inexistant"
            });
        }

        // 7) Si on arrive ici, l’utilisateur existe
        // On récupère ses données
        const user = results[0];

        // 8) On inverse l’état de présence
        // true devient false, false devient true
        const newPresenceStatus = !user.is_present;

        // 9) Requête SQL pour mettre à jour l’état de présence
        const updateQuery = "UPDATE users SET is_present = ? WHERE id = ?";

        // 10) Exécution de la mise à jour
        db.query(updateQuery, [newPresenceStatus, user.id], (updateErr) => {

            // 11) Gestion d’erreur lors de la mise à jour
            if (updateErr) {
                return res.status(500).json({
                    status: "error",
                    message: "Erreur lors de la mise à jour de la présence"
                });
            }

            // 12) Tout s’est bien passé
            // On renvoie une réponse claire au client
            return res.status(200).json({
                status: "success",
                user: {
                    id: user.id,
                    full_name: user.full_name,
                    is_present: newPresenceStatus
                }
            });
        });
    });
};
