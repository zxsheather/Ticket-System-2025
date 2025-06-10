const express = require('express');
const router = express.Router();
const ticketController = require('../controllers/ticketController');

// 添加车次
router.post('/add', async (req, res) => {
  try {
    if (!req.session.user || req.session.user.privilege < 5) {
      return res.status(403).json({
        success: false,
        message: '权限不足'
      });
    }
    
    const {
      trainId, stationNum, seatNum, stations, prices,
      startTime, travelTimes, stopoverTimes, saleDate, type
    } = req.body;
    
    const result = await ticketController.addTrain(
      trainId,
      parseInt(stationNum),
      parseInt(seatNum),
      stations,
      prices.map(p => parseInt(p)),
      startTime,
      travelTimes.map(t => parseInt(t)),
      stopoverTimes ? stopoverTimes.map(t => parseInt(t)) : [],
      saleDate,
      type
    );
    
    res.json(result);
  } catch (error) {
    res.status(500).json({
      success: false,
      message: '服务器错误'
    });
  }
});

// 删除车次
router.delete('/:trainId', async (req, res) => {
  try {
    if (!req.session.user || req.session.user.privilege < 5) {
      return res.status(403).json({
        success: false,
        message: '权限不足'
      });
    }
    
    const { trainId } = req.params;
    const result = await ticketController.deleteTrain(trainId);
    
    res.json(result);
  } catch (error) {
    res.status(500).json({
      success: false,
      message: '服务器错误'
    });
  }
});

// 发布车次
router.post('/:trainId/release', async (req, res) => {
  try {
    if (!req.session.user || req.session.user.privilege < 5) {
      return res.status(403).json({
        success: false,
        message: '权限不足'
      });
    }
    
    const { trainId } = req.params;
    const result = await ticketController.releaseTrain(trainId);
    
    res.json(result);
  } catch (error) {
    res.status(500).json({
      success: false,
      message: '服务器错误'
    });
  }
});

// 查询车次
router.get('/:trainId', async (req, res) => {
  try {
    const { trainId } = req.params;
    const { date } = req.query;
    
    if (!date) {
      return res.status(400).json({
        success: false,
        message: '日期参数缺失'
      });
    }
    
    const result = await ticketController.queryTrain(trainId, date);
    
    res.json(result);
  } catch (error) {
    res.status(500).json({
      success: false,
      message: '服务器错误'
    });
  }
});

module.exports = router;
