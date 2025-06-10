const express = require('express');
const router = express.Router();
const ticketController = require('../controllers/ticketController');

// 中间件：检查管理员权限
const requireAdmin = (req, res, next) => {
  if (!req.session.user || req.session.user.privilege < 5) {
    return res.status(403).json({
      success: false,
      message: '权限不足'
    });
  }
  next();
};

// 添加车次
router.post('/train/add', requireAdmin, async (req, res) => {
  try {
    const {
      trainId, stationNum, seatNum, stations, prices,
      startTime, travelTimes, stopoverTimes, saleDate, type
    } = req.body;
    
    if (!trainId || !stationNum || !seatNum || !stations || !prices ||
        !startTime || !travelTimes || !saleDate || !type) {
      return res.status(400).json({
        success: false,
        message: '车次信息不完整'
      });
    }
    
    const result = await ticketController.addTrain(
      trainId,
      stationNum,
      seatNum,
      stations,
      prices,
      startTime,
      travelTimes,
      stopoverTimes || [],
      saleDate,
      type
    );
    
    if (result.success) {
      res.json({
        success: true,
        message: '添加车次成功'
      });
    } else {
      res.json({
        success: false,
        message: '添加车次失败，车次可能已存在'
      });
    }
  } catch (error) {
    res.status(500).json({
      success: false,
      message: '服务器错误'
    });
  }
});

// 删除车次
router.delete('/train/:trainId', requireAdmin, async (req, res) => {
  try {
    const { trainId } = req.params;
    
    const result = await ticketController.deleteTrain(trainId);
    
    if (result.success) {
      res.json({
        success: true,
        message: '删除车次成功'
      });
    } else {
      res.json({
        success: false,
        message: '删除车次失败，车次可能不存在或已发布'
      });
    }
  } catch (error) {
    res.status(500).json({
      success: false,
      message: '服务器错误'
    });
  }
});

// 发布车次
router.post('/train/:trainId/release', requireAdmin, async (req, res) => {
  try {
    const { trainId } = req.params;
    
    const result = await ticketController.releaseTrain(trainId);
    
    if (result.success) {
      res.json({
        success: true,
        message: '发布车次成功'
      });
    } else {
      res.json({
        success: false,
        message: '发布车次失败，车次可能不存在或已发布'
      });
    }
  } catch (error) {
    res.status(500).json({
      success: false,
      message: '服务器错误'
    });
  }
});

// 查询车次信息
router.get('/train/:trainId', requireAdmin, async (req, res) => {
  try {
    const { trainId } = req.params;
    const { date } = req.query;
    
    if (!date) {
      return res.status(400).json({
        success: false,
        message: '日期不能为空'
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

// 系统状态查询
router.get('/status', requireAdmin, (req, res) => {
  res.json({
    success: true,
    status: {
      webServer: 'running',
      backendProcess: ticketController.isRunning ? 'running' : 'stopped',
      uptime: process.uptime(),
      memory: process.memoryUsage(),
      timestamp: new Date().toISOString()
    }
  });
});

// 重启后端服务
router.post('/restart-backend', requireAdmin, async (req, res) => {
  try {
    ticketController.cleanup();
    
    setTimeout(() => {
      ticketController.startBackend();
    }, 2000);
    
    res.json({
      success: true,
      message: '后端服务重启中...'
    });
  } catch (error) {
    res.status(500).json({
      success: false,
      message: '重启失败'
    });
  }
});

module.exports = router;
