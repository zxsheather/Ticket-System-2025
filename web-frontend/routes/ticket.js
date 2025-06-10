const express = require('express');
const router = express.Router();
const ticketController = require('../controllers/ticketController');

// 查询车票
router.get('/search', async (req, res) => {
  try {
    const { from, to, date, sort = 'time' } = req.query;
    
    if (!from || !to || !date) {
      return res.status(400).json({
        success: false,
        message: '出发地、目的地和日期不能为空'
      });
    }
    
    const result = await ticketController.queryTicket(from, to, date, sort);
    
    res.json(result);
  } catch (error) {
    res.status(500).json({
      success: false,
      message: '服务器错误'
    });
  }
});

// 查询换乘
router.get('/transfer', async (req, res) => {
  try {
    const { from, to, date, sort = 'time' } = req.query;
    
    if (!from || !to || !date) {
      return res.status(400).json({
        success: false,
        message: '出发地、目的地和日期不能为空'
      });
    }
    
    const result = await ticketController.queryTransfer(from, to, date, sort);
    
    res.json(result);
  } catch (error) {
    res.status(500).json({
      success: false,
      message: '服务器错误'
    });
  }
});

// 获取热门站点（模拟数据）
router.get('/popular-stations', (req, res) => {
  const popularStations = [
    '北京', '上海', '广州', '深圳', '杭州', '南京', '天津', '重庆',
    '成都', '西安', '武汉', '长沙', '郑州', '济南', '沈阳', '大连',
    '青岛', '厦门', '苏州', '无锡', '宁波', '温州', '福州', '合肥',
    '南昌', '长春', '哈尔滨', '石家庄', '太原', '呼和浩特'
  ];
  
  res.json({
    success: true,
    stations: popularStations
  });
});

module.exports = router;
