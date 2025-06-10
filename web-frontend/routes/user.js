const express = require('express');
const router = express.Router();
const ticketController = require('../controllers/ticketController');

// 登录
router.post('/login', async (req, res) => {
  try {
    const { username, password } = req.body;
    
    if (!username || !password) {
      return res.status(400).json({
        success: false,
        message: '用户名和密码不能为空'
      });
    }
    
    const result = await ticketController.login(username, password);
    
    if (result.success) {
      // 获取用户信息
      const profileResult = await ticketController.queryProfile(username, username);
      
      if (profileResult.success) {
        req.session.user = {
          username: username,
          ...profileResult.profile
        };
        
        res.json({
          success: true,
          message: '登录成功',
          user: req.session.user
        });
      } else {
        res.json({
          success: false,
          message: '登录成功但获取用户信息失败'
        });
      }
    } else {
      res.json({
        success: false,
        message: '用户名或密码错误'
      });
    }
  } catch (error) {
    res.status(500).json({
      success: false,
      message: '服务器错误'
    });
  }
});

// 登出
router.post('/logout', async (req, res) => {
  try {
    if (req.session.user) {
      await ticketController.logout(req.session.user.username);
      req.session.destroy();
    }
    
    res.json({
      success: true,
      message: '登出成功'
    });
  } catch (error) {
    res.status(500).json({
      success: false,
      message: '服务器错误'
    });
  }
});

// 注册
router.post('/register', async (req, res) => {
  try {
    const { username, password, name, mailAddr, privilege = 1 } = req.body;
    
    if (!username || !password || !name || !mailAddr) {
      return res.status(400).json({
        success: false,
        message: '所有字段都是必填的'
      });
    }
    
    // 如果是第一个用户，权限设为10（管理员）
    const actualPrivilege = privilege;
    
    const result = await ticketController.addUser(
      '', // 第一个用户不需要当前用户
      username,
      password,
      name,
      mailAddr,
      actualPrivilege
    );
    
    if (result.success) {
      res.json({
        success: true,
        message: '注册成功'
      });
    } else {
      res.json({
        success: false,
        message: '注册失败，用户名可能已存在'
      });
    }
  } catch (error) {
    res.status(500).json({
      success: false,
      message: '服务器错误'
    });
  }
});

// 获取当前用户信息
router.get('/profile', async (req, res) => {
  try {
    if (!req.session.user) {
      return res.status(401).json({
        success: false,
        message: '未登录'
      });
    }
    
    const result = await ticketController.queryProfile(
      req.session.user.username,
      req.session.user.username
    );
    
    if (result.success) {
      res.json({
        success: true,
        profile: result.profile
      });
    } else {
      res.json({
        success: false,
        message: '获取用户信息失败'
      });
    }
  } catch (error) {
    res.status(500).json({
      success: false,
      message: '服务器错误'
    });
  }
});

// 修改用户信息
router.post('/profile', async (req, res) => {
  try {
    if (!req.session.user) {
      return res.status(401).json({
        success: false,
        message: '未登录'
      });
    }
    
    const { password, name, mailAddr } = req.body;
    
    const result = await ticketController.modifyProfile(
      req.session.user.username,
      req.session.user.username,
      password,
      name,
      mailAddr
    );
    
    if (result.success) {
      // 更新会话中的用户信息
      req.session.user = {
        ...req.session.user,
        ...result.profile
      };
      
      res.json({
        success: true,
        message: '修改成功',
        profile: result.profile
      });
    } else {
      res.json({
        success: false,
        message: '修改失败'
      });
    }
  } catch (error) {
    res.status(500).json({
      success: false,
      message: '服务器错误'
    });
  }
});

// 查询其他用户信息（管理员功能）
router.get('/profile/:username', async (req, res) => {
  try {
    if (!req.session.user || req.session.user.privilege < 5) {
      return res.status(403).json({
        success: false,
        message: '权限不足'
      });
    }
    
    const { username } = req.params;
    
    const result = await ticketController.queryProfile(
      req.session.user.username,
      username
    );
    
    if (result.success) {
      res.json({
        success: true,
        profile: result.profile
      });
    } else {
      res.json({
        success: false,
        message: '用户不存在'
      });
    }
  } catch (error) {
    res.status(500).json({
      success: false,
      message: '服务器错误'
    });
  }
});

// 添加用户（管理员功能）
router.post('/add', async (req, res) => {
  try {
    if (!req.session.user || req.session.user.privilege < 5) {
      return res.status(403).json({
        success: false,
        message: '权限不足'
      });
    }
    
    const { username, password, name, mailAddr, privilege } = req.body;
    
    if (!username || !password || !name || !mailAddr) {
      return res.status(400).json({
        success: false,
        message: '所有字段都是必填的'
      });
    }
    
    const result = await ticketController.addUser(
      req.session.user.username,
      username,
      password,
      name,
      mailAddr,
      privilege || 1
    );
    
    if (result.success) {
      res.json({
        success: true,
        message: '添加用户成功'
      });
    } else {
      res.json({
        success: false,
        message: '添加用户失败'
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
