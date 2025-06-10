const express = require('express');
const router = express.Router();
const ticketController = require('../controllers/ticketController');

// 购票
router.post('/buy', async (req, res) => {
  try {
    if (!req.session.user) {
      return res.status(401).json({
        success: false,
        message: '请先登录'
      });
    }
    
    const { trainId, date, num, from, to, queue = false } = req.body;
    
    if (!trainId || !date || !num || !from || !to) {
      return res.status(400).json({
        success: false,
        message: '购票信息不完整'
      });
    }
    
    const result = await ticketController.buyTicket(
      req.session.user.username,
      trainId,
      date,
      num,
      from,
      to,
      queue
    );
    
    if (result.success) {
      res.json({
        success: true,
        message: result.queued ? '已加入候补队列' : '购票成功',
        queued: result.queued || false,
        totalPrice: result.totalPrice
      });
    } else {
      res.json({
        success: false,
        message: '购票失败，可能是余票不足'
      });
    }
  } catch (error) {
    res.status(500).json({
      success: false,
      message: '服务器错误'
    });
  }
});

// 查询订单
router.get('/list', async (req, res) => {
  try {
    if (!req.session.user) {
      return res.status(401).json({
        success: false,
        message: '请先登录'
      });
    }
    
    const result = await ticketController.queryOrder(req.session.user.username);
    
    res.json(result);
  } catch (error) {
    res.status(500).json({
      success: false,
      message: '服务器错误'
    });
  }
});

// 退票
router.post('/refund', async (req, res) => {
  try {
    if (!req.session.user) {
      return res.status(401).json({
        success: false,
        message: '请先登录'
      });
    }
    
    const { orderNum = 1 } = req.body;
    
    const result = await ticketController.refundTicket(
      req.session.user.username,
      orderNum
    );
    
    if (result.success) {
      res.json({
        success: true,
        message: '退票成功'
      });
    } else {
      res.json({
        success: false,
        message: '退票失败，可能是订单不存在或已退票'
      });
    }
  } catch (error) {
    res.status(500).json({
      success: false,
      message: '服务器错误'
    });
  }
});

module.exports = router;
