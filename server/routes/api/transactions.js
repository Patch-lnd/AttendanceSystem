const express = require('express');
const router = express.Router();
const db = require('../../db');

// 🔔 Liste des clients connectés via SSE
let clients = [];

// 🔌 Route SSE : connexion côté client (navigateur)
router.get('/events', (req, res) => {
  res.setHeader('Content-Type', 'text/event-stream');
  res.setHeader('Cache-Control', 'no-cache');
  res.setHeader('Connection', 'keep-alive');
  res.flushHeaders();

  clients.push(res);

  req.on('close', () => {
    clients = clients.filter(c => c !== res);
  });
});

// 📢 Fonction pour envoyer un événement à tous les clients connectés
function broadcast(data) {
  clients.forEach(c => c.write(`data: ${JSON.stringify(data)}\n\n`));
}

// GET : afficher le formulaire de transaction vide ou avec messages via query
router.get('/', (req, res) => {
  const { status, message, card_uid, pin, amount } = req.query;

  const finalMessage = status && message
    ? { type: status === 'success' ? 'success' : 'error', text: message }
    : null;

  const autoFill = card_uid && pin && amount
    ? { card_uid, pin, amount }
    : null;

  return res.render('transactions', {
    message: finalMessage,
    autoFill
  });
});

// POST : traiter une transaction
router.post('/', (req, res) => {
  const { card_uid, pin, amount, from } = req.body;

  if (!card_uid || !pin || !amount) {
    return res.render('transactions', {
      message: { type: 'error', text: 'Card UID, PIN, and Amount are required' },
      autoFill: null
    });
  }

  const amountNum = parseFloat(amount);

  try {
    const userQuery = 'SELECT * FROM users WHERE card_uid = ?';
    db.query(userQuery, [card_uid], (err, userRows) => {
      if (err) {
        return res.render('transactions', {
          message: { type: 'error', text: 'Database query error' },
          autoFill: null
        });
      }

      if (userRows.length === 0) {
        if (from === 'esp32') {
          res.json({ status: 'error', message: 'User not found' });
          broadcast({ status: 'error', message: 'User not found' });
          return;
        }

        return res.render('transactions', {
          message: { type: 'error', text: 'User not found' },
          autoFill: null
        });
      }

      const user = userRows[0];
      const receivedPin = (pin || '').trim();
      const storedPin = (user.pin_code || '').toString().trim();

      if (storedPin !== receivedPin) {
        if (from === 'esp32') {
          res.json({ status: 'error', message: 'Invalid PIN' });
          broadcast({ status: 'error', message: 'Invalid PIN' });
          return;
        }

        return res.render('transactions', {
          message: { type: 'error', text: 'Invalid PIN' },
          autoFill: null
        });
      }

      if (user.balance < amountNum) {
        if (from === 'esp32') {
          res.json({ status: 'error', message: 'Insufficient balance' });
          broadcast({ status: 'error', message: 'Insufficient balance' });
          return;
        }

        return res.render('transactions', {
          message: { type: 'error', text: 'Insufficient balance' },
          autoFill: null
        });
      }

      const transactionQuery = 'INSERT INTO transactions (user_id, amount, transaction_type) VALUES (?, ?, ?)';
      db.query(transactionQuery, [user.id, amountNum, 'debit'], (err) => {
        if (err) {
          if (from === 'esp32') {
            res.json({ status: 'error', message: 'Error inserting transaction' });
            broadcast({ status: 'error', message: 'Error inserting transaction' });
            return;
          }

          return res.render('transactions', {
            message: { type: 'error', text: 'Error inserting transaction' },
            autoFill: null
          });
        }

        const updateBalanceQuery = 'UPDATE users SET balance = balance - ? WHERE id = ?';
        db.query(updateBalanceQuery, [amountNum, user.id], (err) => {
          if (err) {
            if (from === 'esp32') {
              res.json({ status: 'error', message: 'Error updating balance' });
              broadcast({ status: 'error', message: 'Error updating balance' });
              return;
            }

            return res.render('transactions', {
              message: { type: 'error', text: 'Error updating balance' },
              autoFill: null
            });
          }

          if (from === 'esp32') {
            res.json({ status: 'success', message: 'Transaction Success' });
            broadcast({ status: 'success', message: 'Transaction effectuée avec succès via ESP32' });
            return;
          }

          return res.render('transactions', {
            message: { type: 'success', text: 'Transaction Success' },
            autoFill: null
          });
        });
      });
    });
  } catch (error) {
    console.error('Error processing transaction:', error);
    return res.render('transactions', {
      message: { type: 'error', text: 'Internal Server Error' },
      autoFill: null
    });
  }
});

module.exports = router;
